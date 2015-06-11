/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

// temporarily global; will home these when rationalize object lifetimes
InstructionStats *instructionStats=NULL;
bool traceBB=false;

void PrintInstructionTrace() {
    if ((instructionStats!=NULL)&&traceBB)
        instructionStats->PrintInstructionTrace();
}

int __cdecl compareOpCounts(const void *a,const void *b) {
    OpCount *oa=(OpCount*)a;
    OpCount *ob=(OpCount*)b;

    return(ob->count-oa->count);
}

    // TODO: remove nRegs arg in favor of using fn->Locals
ControlFlowGraph *RTLGenerator::TranslateFunction(Js::FunctionBody *fn,int nRegs) {
    this->nRegs=nRegs;
    argCountCurrentFunction=0;
    this->fn=fn;
    instructionCount=0;
    m_reader.Create(fn);
    functionStartOffset=m_reader.GetCurrentOffset();
    relocs=ListFn<Reloc*>::MakeListHead(alloc);
    cfg=new ControlFlowGraph(alloc);
    // add room for a pointer at the end (to resolve relocations for branches to the end
    int nBytes=m_reader.GetSizeInBytes();
    offsetToInstruction=(Instruction**)(alloc->AllocZero(nBytes*sizeof(Instruction*)));
    registerDescriptions=(RegisterDescription**)(alloc->Alloc(nRegs*sizeof(RegisterDescription*)));
    for (Js::RegSlot i=0;i<nRegs;i++) {
        registerDescriptions[i]=Anew(alloc,RegisterDescription,alloc,i);
    }
    currentBasicBlock=cfg->GetEntry();
    BuildCFG();
    Instruction *functionExitInstruction=MakeStructZ(alloc,Instruction);
    functionExitInstruction->op=Js::OpCode::FunctionExit;
    currentBasicBlock->AddSuccessor(cfg->GetExit());
    cfg->GetExit()->AddInstruction(functionExitInstruction);
    ApplyRelocs();
    CharacterizeRegisters(nRegs);
#define REGULAR
#ifdef REGULAR
    AssignMachineRegistersX86(nRegs);
    if (!silent)
        cfg->Print(memContext);
    EmitFunctionX86(cfg,nRegs);
#else 
    EmitFunctionUsingDagsX86(cfg,nRegs);
#endif
    fn->NativeEntryPoint = codeBuffer;
    return cfg;
}

bool RTLGenerator::LegalRegisterAssignments(Instruction *instruction) {
    int rs[3];
    int mrs[3];
    int nsrc=0;

    if ((instruction->rs1!=ReturnRegister)&&
        (!(RegisterDescription::IsConstRegType(registerDescriptions[instruction->rs1]->GetRegisterType())))) {
            rs[nsrc]=instruction->rs1;
            mrs[nsrc]=instruction->mrs1;
            nsrc++;
    }

    if ((instruction->rs2!=ReturnRegister)&&
        (!(RegisterDescription::IsConstRegType(registerDescriptions[instruction->rs2]->GetRegisterType())))) {
            rs[nsrc]=instruction->rs2;
            mrs[nsrc]=instruction->mrs2;
            nsrc++;
    }


    if (instruction->op==Js::OpCode::StElemI_A) {
        StElemInstruction *stElemInstruction=(StElemInstruction*)instruction;
        if ((stElemInstruction->val!=ReturnRegister)&&
            (!(RegisterDescription::IsConstRegType(registerDescriptions[stElemInstruction->val]->GetRegisterType())))) {
                rs[nsrc]=stElemInstruction->val;
                mrs[nsrc]=stElemInstruction->mval;
                nsrc++;
        }
    }

    for (int i=0;i<nsrc;i++) {
        for (int j=i+1;j<nsrc;j++) {
            if (mrs[i]==mrs[j]) {
                if (rs[i]!=rs[j])
                    return false;
            }
        }
    }

    return true;
}


void RTLGenerator::AssignMachineRegisters(List<Instruction*> *entry,MachineRegisters *machineRegisters,Instruction **prefix) {
    Instruction *instruction=entry->data;
    if (instruction->rs1!=ReturnRegister) {
        instruction->mrs1=AssignSourceRegister(entry,instruction,instruction->rs1,machineRegisters,
            &prefix[0]);
    }
    if (instruction->rs2!=ReturnRegister) {
        instruction->mrs2=AssignSourceRegister(entry,instruction,instruction->rs2,machineRegisters,
            &prefix[1]);
    }
    if (instruction->op==Js::OpCode::StElemI_A) {
        StElemInstruction *stElemInstruction=(StElemInstruction*)instruction;
        if (stElemInstruction->val!=ReturnRegister) {
            stElemInstruction->mval=AssignSourceRegister(entry,instruction,stElemInstruction->val,machineRegisters,
                &prefix[2]);

        }
    }
    FreeIfNotUsed(machineRegisters,instruction->rs1,instruction);
    if (instruction->rs1!=instruction->rs2)
        FreeIfNotUsed(machineRegisters,instruction->rs2,instruction);
    if (instruction->op==Js::OpCode::StElemI_A) {
        StElemInstruction *stElemInstruction=(StElemInstruction*)instruction;
        if ((stElemInstruction->val!=instruction->rs1)&&(stElemInstruction->val!=instruction->rs2))
            FreeIfNotUsed(machineRegisters,stElemInstruction->val,instruction);
    }
    if (instruction->rd!=ReturnRegister) {
        instruction->mrd=AssignDestinationRegister(entry,instruction,instruction->rd,machineRegisters,&prefix[3]);
    }
    AssertMsg(LegalRegisterAssignments(instruction),"Illegal register assignment");
}

void RTLGenerator::AssignMachineRegisters(BasicBlock *block,MachineRegisters *machineRegisters,int nRegs) {
    // assign ordinals to instructions
    block->NumberInstructions();
    List<Instruction*> *instructions=block->GetInstructions();
    for (List<Instruction*> *entry=instructions->next;!(entry->isHead);entry=entry->next) {
        Instruction *prefixInstrs[4];  // rs1, rs2, val, rd
        for (int i=0;i<4;i++)
            prefixInstrs[i]=NULL;
        AssignMachineRegisters(entry,machineRegisters,prefixInstrs);
        for (int i=0;i<4;i++) {
            Instruction *prefix=prefixInstrs[i];
            if (prefix!=NULL) {
                // if there is a spill or reload instruction, insert it before the current instruction
                List<Instruction*> *prefixEntry=ListFn<Instruction*>::MakeListEntry(alloc);
                prefixEntry->data=prefix;
                prefix->basicBlock=block;
                ListFn<Instruction*>::InsertBefore(entry,prefixEntry);
                instructionCount++;
            }
        }
    }
    if (!ListFn<Instruction*>::Empty(instructions)) {
        Instruction *lastInstruction=instructions->prev->data;
        machineRegisters->HomeAndFreeAll(lastInstruction->isBranchInstruction,block,block->GetInstructions()->prev,registerDescriptions);
        for (int i=0;i<nRegs;i++) {
            registerDescriptions[i]->ClearLocation();
        }
    }
}

void RTLGenerator::AssignMachineRegistersX86(int nRegs) {
    MachineRegisters *x86MachineRegisters=Anew(alloc,MachineRegisters,alloc,X86_NINTREGS,X86_TEMP_REGISTER,X86_EDX,X86_FRAME_POINTER,
        X86_STACK_POINTER,X86_CALLEE_SAVED);
    List<BasicBlock*> *blocks=cfg->GetBlocks();
    for (List<BasicBlock*> *entry=blocks->next;!(entry->isHead);entry=entry->next) {
        AssignMachineRegisters(entry->data,x86MachineRegisters,nRegs);
    }
}
 
void RTLGenerator::CharacterizeRegisters(int nRegs) {
    registerDescriptions[0]->SetTemp();
    for (int i=1;i<nRegs;i++) {
        registerDescriptions[i]->Characterize(); 
    }
}

void RTLGenerator::DescribeUses(Instruction *instruction) {
    if (instruction->rs1!=ReturnRegister) {
        RegisterDescription *descr=registerDescriptions[instruction->rs1];
        descr->AddUse(instruction);
    }
    if (instruction->rs2!=ReturnRegister) {
        RegisterDescription *descr=registerDescriptions[instruction->rs2];
        descr->AddUse(instruction);
    }
    if (instruction->op==Js::OpCode::StElemI_A) {
        StElemInstruction *stElemInstruction=(StElemInstruction*)instruction;
        if (stElemInstruction->val!=ReturnRegister) {
            RegisterDescription *descr=registerDescriptions[stElemInstruction->val];
            descr->AddUse(stElemInstruction);
        }
    }
}

void GenerateFunction(Js::FunctionBody *fn,ArenaAllocator *alloc,int nRegs,Js::Configuration *config) {
    if(config->flags->IsEnabled(Js::ProfileFlag))
    {
        config->profiler->Begin(Js::BackEndPhase);
    }
    traceBB=config->flags->ITrace;
    if ((instructionStats==NULL)&&traceBB)
        instructionStats=Anewz(alloc,InstructionStats,alloc);
    RTLGenerator *gen=Anewz(alloc,RTLGenerator,fn->GetMemoryContext(),alloc,!config->flags->Verbose);
    gen->TranslateFunction(fn,nRegs);
    if(config->flags->IsEnabled(Js::ProfileFlag))
    {
        config->profiler->End(Js::BackEndPhase);
    }
}

void RTLGenerator::ApplyRelocs() {
    for (List<Reloc*> *entry=relocs->next;!(entry->isHead);entry=entry->next) {
        Reloc *reloc=entry->data;
        AssertMsg(reloc->relocType==RelocTypeBranch,"Unrecognized reloc type");  // currently the only reloc type
        BranchInstruction *branchInstruction=reinterpret_cast<BranchInstruction*>(reloc->consumer);
        Instruction *targetInstruction=GetInstructionFromOffset(reloc->offset);
        branchInstruction->branchTarget=targetInstruction->basicBlock;
        AssertMsg(branchInstruction->branchTarget!=NULL,"No Basic block at branch target offset");
    }
}

void RTLGenerator::CloseBasicBlock(bool canFallThrough,unsigned int targetOffset) {
    Instruction *targetInstruction=GetInstructionFromOffset(targetOffset);
    BasicBlock *targetBlock;
    if (targetInstruction==NULL) {
        // forward branch
        targetBlock=Anew(alloc,BasicBlock,alloc,targetOffset);
        targetInstruction=MakeStructZ(alloc,Instruction);
        targetInstruction->op=Js::OpCode::BlockStart;
        targetInstruction->basicBlock=targetBlock;
        SetInstructionAtOffset(targetOffset,targetInstruction);
    }
    else {
        targetBlock=targetInstruction->basicBlock;
        BasicBlock *origTargetBlock=targetBlock;
        // split if backward branch into the middle of a block
        targetBlock=targetBlock->SplitIfNecessary(targetInstruction,targetOffset);
        if (targetBlock!=origTargetBlock)
            cfg->InsertBlockInOrder(targetBlock);
    }
    CloseBasicBlock(canFallThrough,targetBlock);
}

void RTLGenerator::CloseBasicBlock(bool canFallThrough,BasicBlock *targetBlock) {
    currentBasicBlock->AddSuccessor(targetBlock);
    NewBasicBlock(canFallThrough);
}

void RTLGenerator::NewBasicBlock(bool canFallThrough) {
    int currentOffset=m_reader.GetCurrentOffset();
    Instruction *nextInstruction=GetInstructionFromOffset(currentOffset);
    BasicBlock *nextBasicBlock;
    if (nextInstruction==NULL) {
        nextBasicBlock=Anew(alloc,BasicBlock,alloc,currentOffset);
    }
    else nextBasicBlock=nextInstruction->basicBlock;
    if (canFallThrough)
        currentBasicBlock->AddSuccessor(nextBasicBlock);
    cfg->AddBlock(nextBasicBlock);
    currentBasicBlock=nextBasicBlock;
}

void RTLGenerator::AddConditionalBranchInstruction(BranchInstruction *instruction,unsigned int targetOffset) {
    AddInstructionToCurrentBlock(instruction);
    CloseBasicBlock(true,targetOffset);
}

void RTLGenerator::AddBranchInstruction(BranchInstruction *instruction,unsigned int targetOffset) {
    AddInstructionToCurrentBlock(instruction);
    CloseBasicBlock(false,targetOffset);
}

Instruction *RTLGenerator::ConditionalBranch(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::BrC, "Validate correct layout"); 
    const unaligned Js::OpLayoutBrC *conditionalBranchInsn = m_reader.BrC(); 
    BranchInstruction *branchInstruction=MakeStructZ(alloc,BranchInstruction);
    branchInstruction->isBranchInstruction=true;
    branchInstruction->rs1=conditionalBranchInsn->R1;
    branchInstruction->rs2=conditionalBranchInsn->R2;
    branchInstruction->op=op;
    AddBranchReloc(conditionalBranchInsn->Offset,branchInstruction);
    AddConditionalBranchInstruction(branchInstruction,conditionalBranchInsn->Offset);
    return branchInstruction;
}

Instruction *RTLGenerator::ConditionalBranchReg1(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::BrB, "Validate correct layout"); 
    const unaligned Js::OpLayoutBrB *conditionalBranchInsn = m_reader.BrB(); 
    BranchInstruction *branchInstruction=MakeStructZ(alloc,BranchInstruction);
    branchInstruction->rs1=conditionalBranchInsn->R1;
    branchInstruction->isBranchInstruction=true;
    branchInstruction->op=op;
    AddBranchReloc(conditionalBranchInsn->Offset,branchInstruction);
    AddConditionalBranchInstruction(branchInstruction,conditionalBranchInsn->Offset);
    return branchInstruction;
}

Instruction *RTLGenerator::ConditionalBranchProperty(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::BrProperty, "Validate correct layout"); 
    const unaligned Js::OpLayoutBrProperty *conditionalBranchInsn = m_reader.BrProperty(); 
    BranchImmInstruction *branchInstruction=MakeStructZ(alloc,BranchImmInstruction);
    branchInstruction->rs1=conditionalBranchInsn->Instance;
    branchInstruction->immInt=conditionalBranchInsn->PropertyId;
    branchInstruction->isBranchInstruction=true;
    branchInstruction->op=op;
    AddBranchReloc(conditionalBranchInsn->Offset,branchInstruction);
    AddConditionalBranchInstruction(branchInstruction,conditionalBranchInsn->Offset);
    return branchInstruction;
}

Instruction *RTLGenerator::Branch(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Br, "Validate correct layout"); 
    const unaligned Js::OpLayoutBr *branchInsn = m_reader.Br(); 
    BranchInstruction *branchInstruction=MakeStructZ(alloc,BranchInstruction);
    branchInstruction->op=op;
    branchInstruction->isBranchInstruction=true;
    AddBranchReloc(branchInsn->Offset,branchInstruction);
    AddBranchInstruction(branchInstruction,branchInsn->Offset);
    return branchInstruction;
}

Instruction *RTLGenerator::ReturnInstruction(Js::OpCode op) {
    BasicBlock *exitBlock=cfg->GetExit();
    BranchInstruction *branchInstruction=MakeStructZ(alloc,BranchInstruction);
    branchInstruction->isBranchInstruction=true;
    branchInstruction->op=op;
    branchInstruction->branchTarget=exitBlock;
    AddInstructionToCurrentBlock(branchInstruction);
    CloseBasicBlock(false,exitBlock);
    return branchInstruction;
}

Instruction *RTLGenerator::StartCallInstruction(Js::OpCode op) {
    IntConstInstruction *intConstInstruction=MakeStructZ(alloc,IntConstInstruction);
    intConstInstruction->op=op;
    const unaligned Js::OpLayoutI4 *startCallInsn = m_reader.I4();
    intConstInstruction->immInt=startCallInsn->C1;
    AddInstructionToCurrentBlock(intConstInstruction);
    return intConstInstruction;
}

Instruction *RTLGenerator::RegisterOperation3(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Reg3, "Validate correct layout");
    const unaligned Js::OpLayoutReg3 *registerInstruction= m_reader.Reg3(); 
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=registerInstruction->R0;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    instruction->rs1=registerInstruction->R1;
    instruction->rs2=registerInstruction->R2;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::RegisterOperation2(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Reg2, "Validate correct layout");
    const unaligned Js::OpLayoutReg2 *registerInstruction= m_reader.Reg2(); 
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=registerInstruction->R0;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    instruction->rs1=registerInstruction->R1;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}


Instruction *RTLGenerator::NewRegEx(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Regex, "Validate correct layout");
    const unaligned Js::OpLayoutRegex *insn= m_reader.Regex(); 
    RegexInstruction *regexInstruction=MakeStructZ(alloc,RegexInstruction);
    regexInstruction->op=op;
    regexInstruction->rd=insn->R0;
    regexInstruction->regularExpression=insn->regularExpression;
    AddInstructionToCurrentBlock(regexInstruction);
    GetRegisterDescription(regexInstruction->rd)->AddDef(regexInstruction);
    return regexInstruction;
}

Instruction *RTLGenerator::LoadConstant(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Reg1, "Validate correct layout");
    const unaligned Js::OpLayoutReg1 *insn= m_reader.Reg1(); 
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=insn->R0;
    if (op==Js::OpCode::LdC_A_I4_1) {
        GetRegisterDescription(instruction->rd)->SetConstant(1,instruction);
    }
    else if (op==Js::OpCode::LdC_A_I4_0) {
        GetRegisterDescription(instruction->rd)->SetConstant(0,instruction);
    }
    else if (op==Js::OpCode::LdC_A_R8_0) {
        GetRegisterDescription(instruction->rd)->SetConstant(0.0,instruction);
    }
    else GetRegisterDescription(instruction->rd)->SetConstant(instruction);
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LoadRoot(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Reg1, "Validate correct layout");
    const unaligned Js::OpLayoutReg1 *insn= m_reader.Reg1(); 
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=insn->R0;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LoadEnv(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::Reg1, "Validate correct layout");
    const unaligned Js::OpLayoutReg1 *insn= m_reader.Reg1(); 
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=insn->R0;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LoadConstantI4(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::I4, "Validate correct layout");
    const unaligned Js::OpLayoutI4 *insn= m_reader.I4(); 
    IntConstInstruction *instruction=MakeStructZ(alloc,IntConstInstruction);
    instruction->op=op;
    instruction->rd=insn->R0;
    instruction->immInt=insn->C1;
    GetRegisterDescription(instruction->rd)->SetConstant(insn->C1,instruction);
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LoadConstantR8(Js::OpCode op) {
    AssertMsg(Js::OpCodeLayouts[op] == Js::OpLayoutType::R8, "Validate correct layout");
    const unaligned Js::OpLayoutR8 *insn= m_reader.R8(); 
    DoubleConstInstruction *instruction=MakeStructZ(alloc,DoubleConstInstruction);
    instruction->op=op;
    instruction->rd=insn->R0;
    instruction->immDouble=insn->C1;
    GetRegisterDescription(instruction->rd)->SetConstant(insn->C1,instruction);
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

// currently only used for LdStr
Instruction *RTLGenerator::LoadAux(Js::OpCode op) {
    const wchar_t * pszContent;
    const unaligned Js::OpLayoutAuxiliary *auxInsn = m_reader.Auxiliary(reinterpret_cast<const byte **>(&pszContent));
    AssertMsg(auxInsn->Size % sizeof(wchar_t) == 0, "Must have full wchar_t data");
    int cchLength = auxInsn->Size / sizeof(wchar_t) - 1;
    AssertMsg(cchLength >= 0, "Must have valid string length");

    LdStrInstruction *instruction=MakeStructZ(alloc,LdStrInstruction);
    instruction->op=op;
    instruction->rd=auxInsn->R0;
    AddInstructionToCurrentBlock(instruction);
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    Js::LiteralString * pstNew = RecyclerNew(memContext->GetRecycler(),Js::LiteralString,pszContent,memContext,cchLength);
    // REVIEW: GC info.
    instruction->str=Js::RecyclableObject::ToAtom(pstNew);
    return instruction;
}


Instruction *RTLGenerator::ArgIn(Js::OpCode op) {
    const unaligned Js::OpLayoutArg *argInsn = m_reader.Arg();
    ArgInstruction *argInstruction=MakeStructZ(alloc,ArgInstruction);
    argInstruction->op=op;
    argInstruction->rd=argInsn->Reg;
    argInstruction->argIndex=argInsn->Arg;
    if (argInstruction->argIndex>=argCountCurrentFunction)
        argCountCurrentFunction=argInstruction->argIndex+1;
    AddInstructionToCurrentBlock(argInstruction);
    GetRegisterDescription(argInsn->Reg)->SetArgIn(argInsn->Arg,argInstruction);
    return argInstruction;
}

Instruction *RTLGenerator::ArgOut(Js::OpCode op) {
    const unaligned Js::OpLayoutArg *argInsn = m_reader.Arg();
    if (op==Js::OpCode::TempArgOut_A) {
        ArgInstruction *argInstruction=MakeStructZ(alloc,ArgInstruction);
        argInstruction->op=op;
        argInstruction->rs1=argInsn->Reg;
        argInstruction->argIndex=argInsn->Arg;
        AddInstructionToCurrentBlock(argInstruction);
        return argInstruction;
    }
    else return NULL;
}

Instruction *RTLGenerator::LdProperty(Js::OpCode op) {
    const unaligned Js::OpLayoutElementC *fieldInsn = m_reader.ElementC();
    IntConstInstruction *instruction=MakeStructZ(alloc,IntConstInstruction);
    instruction->op=op;
    instruction->rs1=fieldInsn->Instance;
    instruction->rd=fieldInsn->Value;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    instruction->immInt=fieldInsn->PropertySlot;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::StProperty(Js::OpCode op) {
    const unaligned Js::OpLayoutElementC *fieldInsn = m_reader.ElementC();
    IntConstInstruction *instruction=MakeStructZ(alloc,IntConstInstruction);
    instruction->op=op;
    instruction->rs1=fieldInsn->Instance;
    instruction->rs2=fieldInsn->Value;
    instruction->immInt=fieldInsn->PropertySlot;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LdElementDirect(Js::OpCode op) {
    const unaligned Js::OpLayoutElementC *fieldInsn = m_reader.ElementC();
    IntConstInstruction *instruction=MakeStructZ(alloc,IntConstInstruction);
    instruction->op=op;
    instruction->rs1=fieldInsn->Instance;
    instruction->rd=fieldInsn->Value;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    instruction->immInt=fieldInsn->PropertySlot;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::StElementDirect(Js::OpCode op) {
    const unaligned Js::OpLayoutElementC *fieldInsn = m_reader.ElementC();
    IntConstInstruction *instruction=MakeStructZ(alloc,IntConstInstruction);
    instruction->op=op;
    instruction->rs1=fieldInsn->Instance;
    instruction->rs2=fieldInsn->Value;
    instruction->immInt=fieldInsn->PropertySlot;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::LdElement(Js::OpCode op) {
    const unaligned Js::OpLayoutElementI *elementInsn = m_reader.ElementI();
    Instruction *instruction=MakeStructZ(alloc,Instruction);
    instruction->op=op;
    instruction->rd=elementInsn->Value;
    GetRegisterDescription(instruction->rd)->AddDef(instruction);
    instruction->rs1=elementInsn->Instance;
    instruction->rs2=elementInsn->Element;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::StElement(Js::OpCode op) {
    const unaligned Js::OpLayoutElementI *elementInsn = m_reader.ElementI();
    StElemInstruction *instruction=MakeStructZ(alloc,StElemInstruction);
    instruction->op=op;
    instruction->val=elementInsn->Value;
    instruction->rs1=elementInsn->Instance;
    instruction->rs2=elementInsn->Element;
    AddInstructionToCurrentBlock(instruction);
    return instruction;
}

Instruction *RTLGenerator::CallDirect(Js::OpCode op) {
    const unaligned Js::OpLayoutCall *callInsn = m_reader.Call();
    CallDirectInstruction *callInstruction=MakeStructZ(alloc,CallDirectInstruction);
    callInstruction->op=op;
    callInstruction->fnBod=callInsn->functionBody;
    callInstruction->nArgs=callInsn->ArgCount;
    callInstruction->rd=callInsn->Return;
    AddInstructionToCurrentBlock(callInstruction);
    return callInstruction;
}

Instruction *RTLGenerator::CallI(Js::OpCode op) {
    const unaligned Js::OpLayoutCallI *callInsn = m_reader.CallI();
    CallInstruction *callInstruction=MakeStructZ(alloc,CallInstruction);
    callInstruction->op=op;
    callInstruction->nArgs=callInsn->ArgCount;
    callInstruction->rs1=callInsn->Function;
    callInstruction->rd=callInsn->Return;
    GetRegisterDescription(callInstruction->rd)->AddDef(callInstruction);
    AddInstructionToCurrentBlock(callInstruction);
    return callInstruction;
}

Instruction *RTLGenerator::CallIM(Js::OpCode op) {
    const unaligned Js::OpLayoutCallIM *callInsn = m_reader.CallIM();
    CallIMInstruction *callInstruction=MakeStructZ(alloc,CallIMInstruction);
    callInstruction->op=op;
    callInstruction->nArgs=callInsn->ArgCount;
    callInstruction->rs1=callInsn->Instance;
    callInstruction->rd=callInsn->Return;
    GetRegisterDescription(callInstruction->rd)->AddDef(callInstruction);
    callInstruction->memberId=callInsn->Member;
    AddInstructionToCurrentBlock(callInstruction);
    return callInstruction;
}

void RTLGenerator::BuildCFG() {
    while (true) {
        int offset=m_reader.GetCurrentOffset();
        Instruction *oldInstruction=GetInstructionFromOffset(offset);
        if (oldInstruction!=NULL) {
            if (oldInstruction->basicBlock!=currentBasicBlock) {
                // fall through to jump target
                currentBasicBlock->AddSuccessor(oldInstruction->basicBlock);
                cfg->AddBlock(oldInstruction->basicBlock);
                currentBasicBlock=oldInstruction->basicBlock;
            }
        }
        Js::OpCode op = m_reader.Op();
        Instruction *instruction=NULL;
        switch (op) {
        case Js::OpCode::EndOfBlock: {
                //
                // Reached an "Js::OpCode::EndOfBlock" so need to exit this translation loop because
                // there is no more byte-code to translate.
                // - This prevents us from accessing random memory as byte-codes.
                // - Functions should contain an "Js::OpCode::Ret" instruction to organize an
                //   orderly return.
                //
                return;
            }

        case Js::OpCode::Nop: {
            break;
        }
        case Js::OpCode::StartCall:
            instruction=StartCallInstruction(op);
            break;
        case Js::OpCode::Ret: {
            instruction=ReturnInstruction(op);
            break;
        }
        case Js::OpCode::BrEq_A: 
        case Js::OpCode::BrNeq_A:
        case Js::OpCode::BrSrEq_A: 
        case Js::OpCode::BrSrNeq_A: 
        case Js::OpCode::BrGt_A: 
        case Js::OpCode::BrLt_A: 
        case Js::OpCode::BrGe_A: 
        case Js::OpCode::BrLe_A: 
            instruction=ConditionalBranch(op);
            break;
        case Js::OpCode::Add_A:
        case Js::OpCode::Div_A:
        case Js::OpCode::Mul_A:
        case Js::OpCode::Rem_A:
        case Js::OpCode::Sub_A:
        case Js::OpCode::And_A:
        case Js::OpCode::Or_A:
        case Js::OpCode::Xor_A:
        case Js::OpCode::Shl_A:
        case Js::OpCode::Shr_A:
        case Js::OpCode::ShrU_A:
        case Js::OpCode::CmEq_A:
        case Js::OpCode::CmSrEq_A:
        case Js::OpCode::CmSrNeq_A:
        case Js::OpCode::CmGt_A:
        case Js::OpCode::CmGe_A:
        case Js::OpCode::CmLt_A:
        case Js::OpCode::CmLe_A:
        case Js::OpCode::CmNeq_A:
        case Js::OpCode::IsInst:
        case Js::OpCode::LdFrameDisplay:
            instruction=RegisterOperation3(op);
            break;
        case Js::OpCode::Call:
            instruction=CallDirect(op);
            break;
        case Js::OpCode::CallI:
        case Js::OpCode::NewScObject:
        case Js::OpCode::NewScObjectSimple:
            instruction=CallI(op);
            break;
        case Js::OpCode::CallIM:
            instruction=CallIM(op);
            break;
        case Js::OpCode::Neg_A:
        case Js::OpCode::Not_A:
        case Js::OpCode::Ld_A:
        case Js::OpCode::LdLen_A:
        case Js::OpCode::NewArray:
        case Js::OpCode::NewScArray:
        case Js::OpCode::GetEnumerator:
        case Js::OpCode::GetCurrent:
        case Js::OpCode::LdHandlerScope:
            instruction=RegisterOperation2(op);
            break;
        case Js::OpCode::Br:
            instruction=Branch(op);
            break;
        case Js::OpCode::LdRoot:
            instruction=LoadRoot(op);
            break;
        case Js::OpCode::LdUndef:
        case Js::OpCode::LdTrue:
        case Js::OpCode::LdFalse:
        case Js::OpCode::LdC_A_Null:
        case Js::OpCode::LdC_A_I4_0:
        case Js::OpCode::LdC_A_I4_1:
        case Js::OpCode::LdC_A_R8_0:
            instruction=LoadConstant(op);
            break;
        case Js::OpCode::LdEnv:
            instruction=LoadEnv(op);
            break;
        case Js::OpCode::BrFalse_A:
        case Js::OpCode::BrTrue_A:
        case Js::OpCode::BrOnEmpty:
            instruction=ConditionalBranchReg1(op);
            break;
        case Js::OpCode::BrOnHasProperty:
        case Js::OpCode::BrOnNoProperty:
            instruction = ConditionalBranchProperty(op);
            break;
        case Js::OpCode::NewRegEx:
            instruction=NewRegEx(op);
            break;
        case Js::OpCode::LdC_A_I4:
            instruction=LoadConstantI4(op);
            break;
        case Js::OpCode::LdC_A_R8:
            instruction=LoadConstantR8(op);
            break;
        case Js::OpCode::ArgIn_A:
            instruction=ArgIn(op);
            break;
        case Js::OpCode::TempArgOut_A:
        case Js::OpCode::ArgOut_A:
            instruction=ArgOut(op);
            break;
        case Js::OpCode::LdFld:
        case Js::OpCode::ScopedLdFld:
        case Js::OpCode::NewScFunc:
            instruction=LdProperty(op);
            break;
        case Js::OpCode::StFld:
        case Js::OpCode::ScopedStFld:
        case Js::OpCode::InitFld:
            instruction=StProperty(op);
            break;
        case Js::OpCode::LdElemC:
            instruction=LdElementDirect(op);
            break;
        case Js::OpCode::StElemC:
            instruction=StElementDirect(op);
            break;
        case Js::OpCode::LdElemI_A:
            instruction=LdElement(op);
            break;
        case Js::OpCode::StElemI_A:
            instruction=StElement(op);
            break;
        case Js::OpCode::LdStr: {
            instruction=LoadAux(op);
            break;
        }
#if DBG_DUMP
        case Js::OpCode::PosInSrc: {
            instruction=LoadConstantI4(op);
            break;
        }
#endif
        default: {
            AssertMsg(false,"Unimplemented opcode");
            break;
        }
        }
        SetInstructionAtOffset(offset,instruction);
    }
}


void RTLGenerator::EmitUnconditionalBranchX86(BranchInstruction *branchInstruction) {
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->relocType=RelocTypeBranch;
    pbuf[0] = I_JMP_PCREL;
    AssertMsg(branchInstruction->branchTarget!=NULL,"UBranch target is null");
    *((unaligned int*)(&pbuf[1])) = (int)branchInstruction->branchTarget;
    reloc->consumerOffset=(unsigned int)((pbuf+1)-codeBuffer);
    pbuf+=5;
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);
}

void RTLGenerator::PushOperandX86(int operand,int machineRegOperand) {
    RegisterDescription *operandDescription=registerDescriptions[operand];
    RegisterType regType=operandDescription->GetRegisterType();
    if ((regType==RegisterTypeIntConstant)||(regType==RegisterTypeDoubleConstant)||(regType==RegisterTypeObjectConstant)) {
        PushConstantAtomX86(operandDescription);
    }
    else *pbuf++ = I_PUSH_REG|(unsigned char)machineRegOperand;
}

void RTLGenerator::LoadOperandX86(int operand,int machineRegOperand,int destReg) {
    RegisterDescription *operandDescription=registerDescriptions[operand];
    RegisterType regType=operandDescription->GetRegisterType();
    if ((regType==RegisterTypeIntConstant)||(regType==RegisterTypeDoubleConstant)||(regType==RegisterTypeObjectConstant)) {
        LoadConstantAtomX86(operandDescription,destReg);
    }

    else if (destReg!=machineRegOperand) {
        // move return value to destination register
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,destReg,machineRegOperand);
    }
}

void RTLGenerator::EmitCompareOpFastPathX86(BranchInstruction *branchInstruction,unsigned char **branchToHelper1Ref,
                                            unsigned char **branchPastHelperRef,int icondOp) {
    LoadOperandX86(branchInstruction->rs1,branchInstruction->mrs1,X86_EDX);
    LoadOperandX86(branchInstruction->rs2,branchInstruction->mrs2,X86_ECX);
    // and ecx,0x2
    *pbuf++ = I_AND_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // and ecx,edx
    *pbuf++ = I_AND_MR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // if both are tagged ints then result will be 0x1
    // go to helper call if not both tagged ints
    *pbuf++ = I_CMP_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    *branchToHelper1Ref=pbuf;
    pbuf+=4;
    // reload right operand into ecx
    LoadOperandX86(branchInstruction->rs2,branchInstruction->mrs2,X86_ECX);
    // compare
    *pbuf++ = I_CMP_RM;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = (unsigned char)icondOp;
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->consumerOffset=(unsigned int)(pbuf-codeBuffer);
    reloc->relocType=RelocTypeBranch;
    *(void **)pbuf=branchInstruction->branchTarget;
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);
    pbuf+=4;
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::EmitConditionalBranchX86(BranchInstruction *branchInstruction,bool twoOperands) {
    bool hasFastPath=false;
    unsigned char *branchToHelper1=NULL;
    unsigned char *branchPastHelper=NULL;
    int icondOp;

    bool pushMemContext=false;
    void *fn=NULL;
    switch (branchInstruction->op) {
        case Js::OpCode::BrFalse_A:
        case Js::OpCode::BrTrue_A:
            fn=Js::JavascriptConversion::ToBoolean;
            pushMemContext=true;
            break;
        case Js::OpCode::BrSrNeq_A:
            fn=Js::JavascriptOperators::NotStrictEqual;
            pushMemContext=true;
            break;
        case Js::OpCode::BrSrEq_A:
            fn=Js::JavascriptOperators::StrictEqual;
            pushMemContext=true;
            break;
        case Js::OpCode::BrOnEmpty:
            fn=Js::JavascriptOperators::OP_BrOnEmpty;
            break;
        case Js::OpCode::BrEq_A: 
            fn=Js::JavascriptOperators::Equal;
            icondOp=I_ICOND_EQ;
            hasFastPath=true;
            pushMemContext=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrNeq_A: 
            fn=Js::JavascriptOperators::NotEqual;
            icondOp=I_ICOND_NE;
            hasFastPath=true;
            pushMemContext=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrGt_A: 
            fn=Js::JavascriptOperators::Greater;
            pushMemContext=true;
            icondOp=I_ICOND_GT;
            hasFastPath=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrLt_A: 
            fn=Js::JavascriptOperators::Less;
            pushMemContext=true;
            icondOp=I_ICOND_LT;
            hasFastPath=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrGe_A: 
            fn=Js::JavascriptOperators::GreaterEqual;
            pushMemContext=true;
            icondOp=I_ICOND_GE;
            hasFastPath=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrLe_A: 
            fn=Js::JavascriptOperators::LessEqual;
            pushMemContext=true;
            icondOp=I_ICOND_LE;
            hasFastPath=true;
            EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        default:
            AssertMsg(false,"Unimplemented conditional branch opcode");
            break;
     }
    if ((hasFastPath)&&(branchToHelper1!=NULL))
        *(unaligned int *)branchToHelper1=(int)(pbuf-(branchToHelper1+4));

    if (pushMemContext) {
        PushImmediateX86((int)memContext);
    }
    // push the comparand(s)
    if (twoOperands) {
        PushOperandX86(branchInstruction->rs2,branchInstruction->mrs2);
    }
    PushOperandX86(branchInstruction->rs1,branchInstruction->mrs1);
    // call the helper
    FinishStdCall(X86_EAX,fn);
    // branch if true
    // cmp
    *pbuf++ = I_CMP_I8_EAX;
    *pbuf++ = 0;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->consumerOffset=(unsigned int)(pbuf-codeBuffer);
    reloc->relocType=RelocTypeBranch;
    *(void **)pbuf=branchInstruction->branchTarget;
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);
    pbuf+=4;
    if (hasFastPath)
        *(unaligned int *)branchPastHelper=(int)(pbuf-(branchPastHelper+4));
}

void RTLGenerator::EmitPropertyBranchX86(BranchImmInstruction *branchInstruction) {
    PushImmediateX86((int)memContext);    
    PushImmediateX86(branchInstruction->immInt);
    PushOperandX86(branchInstruction->rs1,branchInstruction->mrs1);
    FinishStdCall(X86_EAX, Js::JavascriptOperators::HasProperty);
    // cmp
    *pbuf++ = I_CMP_I8_EAX;
    *pbuf++ = 0;
    // branch
    *pbuf++ = I_JCC1;
    if (branchInstruction->op == Js::OpCode::BrOnNoProperty)
    {
        *pbuf++ = I_ICOND_EQ;
    }
    else
    {
        AssertMsg(branchInstruction->op == Js::OpCode::BrOnHasProperty, "Unknown BrProperty opcode");
        *pbuf++ = I_ICOND_NE;
    }
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->consumerOffset=(unsigned int)(pbuf-codeBuffer);
    reloc->relocType=RelocTypeBranch;
    *(void **)pbuf=branchInstruction->branchTarget;
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);
    pbuf+=4;
}

void RTLGenerator::EmitNotAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                          unsigned char **branchPastHelperRef) {
    LoadOperandX86(instruction->rs1,instruction->mrs1,X86_ECX);
    // test ecx,0x1  (check for tag bit)
    *pbuf++ = I_TEST_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_TEST_MI8_RVAL,X86_ECX);
    *pbuf++ = 1;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_Z;
    *branchToHelper1Ref=pbuf;
    pbuf+=4;
    // not ecx
    *pbuf++ = I_NOT;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_NOT_RVAL,X86_ECX);
    // sub ecx,1
    *pbuf++ = I_SUB_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SUB_MI8_RVAL,X86_ECX);
    *pbuf++ = 1;
    // mov mrd,eax
    if (instruction->mrd!=X86_ECX) {
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,instruction->mrd,X86_ECX);
    }
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}


void RTLGenerator::EmitRegisterOperation2X86(Instruction *instruction) {
    bool hasFastPath=false;
    unsigned char *branchToHelper=NULL;
    unsigned char *branchPastHelper=NULL;

    switch (instruction->op) {
        case Js::OpCode::Not_A:
            hasFastPath=true;
            EmitNotAtomFastPathX86(instruction,&branchToHelper,&branchPastHelper);
            break;
    }
    if ((hasFastPath)&&(branchToHelper!=NULL))
        *(unaligned int *)branchToHelper=(int)(pbuf-(branchToHelper+4));
    // call the helper
    void *fn=NULL;
    bool pushMemContext=false;
    switch (instruction->op) {
        case Js::OpCode::Neg_A:
            fn=Js::JavascriptOperators::Negate;
            pushMemContext=true;
            break;
        case Js::OpCode::Not_A:
            fn=Js::JavascriptOperators::Not;
            pushMemContext=true;
            break;
        case Js::OpCode::LdLen_A:
            fn=Js::JavascriptOperators::GetLength;
            break;
        case Js::OpCode::NewArray:
            fn=Js::FixedSizeArray::OP_NewArray;
            pushMemContext=true;
            break;
        case Js::OpCode::NewScArray:
            fn=Js::JavascriptArray::OP_NewScArray;
            pushMemContext=true;
            break;
        case Js::OpCode::GetEnumerator:
            fn=Js::JavascriptOperators::OP_GetEnumerator;
            break;
        case Js::OpCode::GetCurrent:
            pushMemContext=true;
            fn=Js::JavascriptOperators::OP_GetCurrent;
            break;
        case Js::OpCode::LdHandlerScope:
            pushMemContext=true;
            fn=Js::DynamicObject::OP_LdHandlerScope;
            break;
        default:
            AssertMsg(false,"Unimplemented register2 opcode");
            break;
    }
    // some unary operators require the memory context (to create new boxed numbers for example)
    if (pushMemContext)
        PushImmediateX86((int)memContext);
    // push the operand
    PushOperandX86(instruction->rs1,instruction->mrs1);
    FinishStdCall(instruction->mrd,fn);
    if ((hasFastPath)&&(branchPastHelper!=NULL))
        *(unaligned int *)branchPastHelper=(int)(pbuf-(branchPastHelper+4));
}

void RTLGenerator::EmitAndAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelperRef,
                                          unsigned char **branchPastHelperRef) {
    LoadOperandX86(instruction->rs1,instruction->mrs1,X86_ECX);
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_EDX);
    // and ecx, edx
    *pbuf++ = I_AND_MR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // mov edx,ecx
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // and edx,0x1
    *pbuf++ = I_AND_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // skip helper call if both were tagged ints
    *pbuf++ = I_CMP_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    *branchToHelperRef=pbuf;
    pbuf+=4;
    // mov mrd,ecx
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,instruction->mrd,X86_ECX);
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::EmitAddAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                          unsigned char **branchToHelper2Ref,
                                          unsigned char **branchPastHelperRef) {
    LoadOperandX86(instruction->rs1,instruction->mrs1,X86_ECX);
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_EDX);
    // add ecx, edx
    *pbuf++ = I_ADD_MR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // jump to helper on overflow
    *pbuf++ = I_JO_PCREL32_1;
    *pbuf++ = I_JO_PCREL32_2;
    *branchToHelper1Ref=pbuf;
    pbuf+=4;
    // mov edx,ecx
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // and edx,0x2
    *pbuf++ = I_AND_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_EDX);
    *pbuf++ = 0x2;
    // go to helper call if not both tagged ints
    *pbuf++ = I_CMP_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_EDX);
    *pbuf++ = 0x2;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    *branchToHelper2Ref=pbuf;
    pbuf+=4;
    // mov eax,ecx
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EAX,X86_ECX);
    // sub eax,1
    *pbuf++ = I_SUB_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SUB_MI8_RVAL,X86_EAX);
    *pbuf++ = 1;
    // mov mrd,eax
    if (instruction->mrd!=X86_EAX) {
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,instruction->mrd,X86_EAX);
    }
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::EmitShlAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                          unsigned char **branchToHelper2Ref,
                                          unsigned char **branchPastHelperRef) {
    LoadOperandX86(instruction->rs1,instruction->mrs1,X86_EDX);
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_ECX);
    // and ecx,0x2
    *pbuf++ = I_AND_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // and ecx,edx
    *pbuf++ = I_AND_MR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // if both are tagged ints then result will be 0x2
    // go to helper call if not both tagged ints
    *pbuf++ = I_CMP_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    *branchToHelper1Ref=pbuf;
    pbuf+=4;
    // reload right operand into ecx
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_ECX);
    // remove tag bits and adjust ecx to have actual int value
    // shr ecx,2
    *pbuf++ = I_SRL_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SRL_I8_RVAL,X86_ECX);
    *pbuf++ = 2;
    // sub edx,1
    *pbuf++ = I_SUB_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SUB_MI8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // shift left
    *pbuf++ = I_SLL_CL;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SLL_CL_RVAL,X86_EDX);
    // jump to helper on overflow
    *pbuf++ = I_JO_PCREL32_1;
    *pbuf++ = I_JO_PCREL32_2;
    *branchToHelper2Ref=pbuf;
    pbuf+=4;
    // add back tag bit
    // add edx,1
    *pbuf++ = I_ADD_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // mov mrd,edx
    if (instruction->mrd!=X86_EDX) {
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,instruction->mrd,X86_EDX);
    }
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::EmitShrAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                          unsigned char **branchPastHelperRef) {
    LoadOperandX86(instruction->rs1,instruction->mrs1,X86_EDX);
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_ECX);
    // and ecx,0x2
    *pbuf++ = I_AND_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // and ecx,edx
    *pbuf++ = I_AND_MR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EDX,X86_ECX);
    // if both are tagged ints then result will be 0x2
    // go to helper call if not both tagged ints
    *pbuf++ = I_CMP_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_ECX);
    *pbuf++ = 0x1;
    // branch
    *pbuf++ = I_JCC1;
    *pbuf++ = I_ICOND_NE;
    *branchToHelper1Ref=pbuf;
    pbuf+=4;
    // reload right operand into ecx
    LoadOperandX86(instruction->rs2,instruction->mrs2,X86_ECX);
    // remove tag bits and adjust ecx to have actual int value
    // shr ecx,2
    *pbuf++ = I_SRL_I8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SRL_I8_RVAL,X86_ECX);
    *pbuf++ = 2;
    // sub edx,1
    *pbuf++ = I_SUB_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SUB_MI8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // shift right
    *pbuf++ = I_SRA_CL;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_SRA_CL_RVAL,X86_EDX);
    // mask low bits
    *pbuf++ = I_AND_MI32;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI32_RVAL,X86_EDX);
    *(int *)pbuf=0xfffffffc;
    pbuf+=4;
    // add back tag bit
    // add edx,1
    *pbuf++ = I_ADD_MI8;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI8_RVAL,X86_EDX);
    *pbuf++ = 1;
    // mov mrd,edx
    if (instruction->mrd!=X86_EDX) {
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,instruction->mrd,X86_EDX);
    }
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

Js::Atom OP_CmEq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
   return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::Equal(a,b,memContext),memContext);
}

Js::Atom OP_CmNeq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::NotEqual(a,b,memContext),memContext);
}

Js::Atom OP_CmSrEq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
   return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::StrictEqual(a,b,memContext),memContext);
}

Js::Atom OP_CmSrNeq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::NotStrictEqual(a,b,memContext),memContext);
}

Js::Atom OP_CmLt_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::Less(a,b,memContext),memContext);
}

Js::Atom OP_CmLe_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::LessEqual(a,b,memContext),memContext);
}

Js::Atom OP_CmGt_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::Greater(a,b,memContext),memContext);
}

Js::Atom OP_CmGe_A(Js::Atom a,Js::Atom b,MemoryContext* memContext) {
    return Js::JavascriptBoolean::ToAtom(Js::JavascriptOperators::GreaterEqual(a,b,memContext),memContext);
}


void RTLGenerator::EmitRegisterOperation3X86(Instruction *instruction) {
    bool hasFastPath=false;
    unsigned char *branchToHelper1=NULL;
    unsigned char *branchToHelper2=NULL;
    unsigned char *branchPastHelper=NULL;

    switch (instruction->op) {
        case Js::OpCode::And_A: {
            hasFastPath=true;
            EmitAndAtomFastPathX86(instruction,&branchToHelper1,&branchPastHelper);
            break;
        }
        case Js::OpCode::Add_A: {
            hasFastPath=true;
            EmitAddAtomFastPathX86(instruction,&branchToHelper1,&branchToHelper2,&branchPastHelper);
            break;
        case Js::OpCode::Shl_A: {
            hasFastPath=true;
            EmitShlAtomFastPathX86(instruction,&branchToHelper1,&branchToHelper2,&branchPastHelper);
            break;
        }
        case Js::OpCode::Shr_A: {
            hasFastPath=true;
            EmitShrAtomFastPathX86(instruction,&branchToHelper1,&branchPastHelper);
            break;
        }
        }
    }
    if ((hasFastPath)&&(branchToHelper1!=NULL))
        *(unaligned int *)branchToHelper1=(int)(pbuf-(branchToHelper1+4));
    if ((hasFastPath)&&(branchToHelper2!=NULL))
        *(unaligned int *)branchToHelper2=(int)(pbuf-(branchToHelper2+4));
    void *fn=NULL;

    switch (instruction->op) {
        case Js::OpCode::Add_A:
            fn=Js::JavascriptOperators::Add_Full;
            break;
        case Js::OpCode::Div_A:
            fn=Js::JavascriptOperators::Divide;
            break;
        case Js::OpCode::Mul_A:
            fn=Js::JavascriptOperators::Multiply;
            break;
        case Js::OpCode::Rem_A:
            fn=Js::JavascriptOperators::Modulus;
            break;
        case Js::OpCode::Sub_A:
            fn=Js::JavascriptOperators::Subtract;
            break;
        case Js::OpCode::And_A:
            fn=Js::JavascriptOperators::And;
            break;
        case Js::OpCode::Or_A:
            fn=Js::JavascriptOperators::Or;
            break;
        case Js::OpCode::Xor_A:
            fn=Js::JavascriptOperators::Xor;
            break;
        case Js::OpCode::Shl_A:
            fn=Js::JavascriptOperators::ShiftLeft;
            break;
        case Js::OpCode::Shr_A:
            fn=Js::JavascriptOperators::ShiftRight;
            break;
        case Js::OpCode::ShrU_A:
            fn=Js::JavascriptOperators::ShiftRightU;
            break;
        case Js::OpCode::IsInst:
            fn=Js::DynamicObject::OP_IsInst;
            break;
        case Js::OpCode::CmSrEq_A:
            fn=OP_CmSrEq_A;
            break;
        case Js::OpCode::CmSrNeq_A:
            fn=OP_CmSrNeq_A;
            break;
        case Js::OpCode::CmEq_A:
            fn=OP_CmEq_A;
            break;
        case Js::OpCode::CmGt_A:
            fn=OP_CmGt_A;
            break;
        case Js::OpCode::CmGe_A:
            fn=OP_CmGe_A;
            break;
        case Js::OpCode::CmLt_A:
            fn=OP_CmLt_A;
            break;
        case Js::OpCode::CmLe_A:
            fn=OP_CmLe_A;
            break;
        case Js::OpCode::CmNeq_A:
            fn=OP_CmNeq_A;
            break;
        case Js::OpCode::LdFrameDisplay:
            fn=Js::DynamicObject::OP_LdFrameDisplay;
            break;
        default:
            AssertMsg(false,"Unimplemented register3 opcode");
            break;
    }
    // helpers need memory context as third arg
    PushImmediateX86((int)memContext);
    // push the operands right to left
    PushOperandX86(instruction->rs2,instruction->mrs2);
    PushOperandX86(instruction->rs1,instruction->mrs1);
    // call the helper
    FinishStdCall(instruction->mrd,fn);
    if (hasFastPath)
        *(unaligned int *)branchPastHelper=(int)(pbuf-(branchPastHelper+4));
}

void RTLGenerator::LoadConstantAtomX86(RegisterDescription *registerDescription,int reg) {
    void *fn;
    switch (registerDescription->GetConstDefOp()) {
        case Js::OpCode::LdRoot:
            PushImmediateX86((int)memContext);
            fn=Js::DynamicObject::OP_LdRoot;
            FinishStdCall(reg,fn);
            break;
        case Js::OpCode::LdUndef:
            PushImmediateX86((int)memContext);
            fn=Js::DynamicObject::OP_LdUndef;
            FinishStdCall(reg,fn);
            break;
        case Js::OpCode::LdTrue:
            PushImmediateX86((int)memContext);            
            fn=Js::JavascriptBoolean::OP_LdTrue;
            FinishStdCall(reg,fn);
            break;
        case Js::OpCode::LdFalse:
            PushImmediateX86((int)memContext);
            fn=Js::JavascriptBoolean::OP_LdFalse;
            FinishStdCall(reg,fn);
            break;
        case Js::OpCode::LdC_A_Null:
            *pbuf++ = I_MOVDW_REG|(unsigned char)reg;
            *(unaligned Js::Atom *)pbuf=0;
            pbuf+=4;
            break;
        case Js::OpCode::LdC_A_I4_0:
            *pbuf++ = I_MOVDW_REG|(unsigned char)reg;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)0,memContext);
            pbuf+=4;
            break;
        case Js::OpCode::LdC_A_I4_1:
            *pbuf++ = I_MOVDW_REG|(unsigned char)reg;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)1,memContext);
            pbuf+=4;
            break;
        case Js::OpCode::LdC_A_I4:
            *pbuf++ = I_MOVDW_REG|(unsigned char)reg;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)registerDescription->GetIntConst(),memContext);
            pbuf+=4;
            break;
        case Js::OpCode::LdC_A_R8:
        case Js::OpCode::LdC_A_R8_0: {
            // FIX: TODO: GC must not collect until code collected so put on list w/ code
            Js::Atom a=Js::JavascriptNumber::ToAtom(registerDescription->GetDoubleConst(),memContext);
            *pbuf++ = I_MOVDW_REG|(unsigned char)reg;
            *(unaligned Js::Atom *)pbuf=a;
            pbuf+=4;
            break;
        }
    }
}

void RTLGenerator::EmitLoadRootX86(RegisterDescription *registerDescription,int reg) {
    PushImmediateX86((int)memContext);
    void *fn=Js::DynamicObject::OP_LdRoot;
    FinishStdCall(reg,fn);
}

void RTLGenerator::EmitLoadUndefX86(int reg) {
    PushImmediateX86((int)memContext);
    void *fn=Js::DynamicObject::OP_LdUndef;
    FinishStdCall(reg,fn);
}

void RTLGenerator::PushConstantAtomX86(RegisterDescription *registerDescription) {
    switch (registerDescription->GetConstDefOp()) {
        case Js::OpCode::LdRoot:
        case Js::OpCode::LdUndef:
        case Js::OpCode::LdTrue:
        case Js::OpCode::LdFalse:
            LoadConstantAtomX86(registerDescription,X86_EAX);
            *pbuf++ = I_PUSH_EAX;
            break;
        case Js::OpCode::LdC_A_Null:
            *pbuf ++ = I_PUSH_MI32;
            *(unaligned Js::Atom *)pbuf=0;
            pbuf+=sizeof(int);
            break;
        case Js::OpCode::LdC_A_I4_0:
            *pbuf ++ = I_PUSH_MI32;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)0,memContext);
            pbuf+=sizeof(int);
            break;
        case Js::OpCode::LdC_A_I4_1:
            *pbuf ++ = I_PUSH_MI32;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)1,memContext);
            pbuf+=sizeof(int);
            break;
        case Js::OpCode::LdC_A_I4:
            *pbuf ++ = I_PUSH_MI32;
            *(unaligned Js::Atom *)pbuf=Js::JavascriptNumber::ToAtom((int)(registerDescription->GetIntConst()),memContext);
            pbuf+=sizeof(int);
            break;
        case Js::OpCode::LdC_A_R8:
        case Js::OpCode::LdC_A_R8_0:
            // FIX: TODO: GC must not collect until code collected so put on list w/ code
            Js::Atom a=Js::JavascriptNumber::ToAtom(registerDescription->GetDoubleConst(),memContext);
            *pbuf ++ = I_PUSH_MI32;
            *(unaligned Js::Atom *)pbuf=a;
            pbuf+=sizeof(int);
            break;
    }
}

void RTLGenerator::ApplyNativeRelocsX86() {
    for (List<NativeReloc*> *entry=nativeRelocs->next;!(entry->isHead);entry=entry->next) {
        NativeReloc *reloc=entry->data;
        switch (reloc->relocType) {
            case RelocTypeBranch: {
                unaligned BasicBlock *targetBlock= *(unaligned BasicBlock**)(codeBuffer+reloc->consumerOffset);
                int targetOffset=targetBlock->GetNativeStartOffset();
                int pcRel=targetOffset-((int)reloc->consumerOffset+4);  // difference between target and instruction after branch
                *(unaligned int *)(codeBuffer+reloc->consumerOffset)=pcRel;
                break;
            }
            case RelocTypeCallPcrel: {
                unsigned char *targetOffset= *(unaligned unsigned char **)(codeBuffer+reloc->consumerOffset);
                int pcRel=(int)(targetOffset-(codeBuffer+(int)reloc->consumerOffset+4));  // difference between target and instruction after branch
                *(unaligned int *)(codeBuffer+reloc->consumerOffset)=pcRel;
                break;
            }
            case RelocTypeLocalSlotsOffset:
                *(unaligned int *)(codeBuffer+reloc->consumerOffset)=curLocalSlot;
                break;
        }
    }
}

void RTLGenerator::SetMovBaseOps(int rd,int src,int imm,int scale) {
    if (scale != SCALE_1) {
      /* What if the imm is <32 bits, or 0? Do we still need to encode it?
         Yep---x86 doesn't support a smaller encoding.
      */
      AssertMsg(src != X86_REG_NONE,"Need a register to scale"); /* we better have something to scale! */
      *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,rd,RM_IS_SIB);
      *pbuf++ = MakeSIB(scale,src,BASE_IS_NONE);
      *(unaligned int *)pbuf=imm;
      pbuf+=4;
    }
    else if (src==X86_REG_NONE) {
        *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,rd,RM_IS_DIRECT);
        *(unaligned int *)pbuf=imm;
        pbuf+=4;
    }
    else if (X86FitsSimm8(imm)) {
      AssertMsg(src != X86_ESP,"Can't use byte displacement from sp");
      *pbuf++ = MakeModRM(MOD_BYTE_DISPLACEMENT,rd,src);
      *pbuf++ = (char)imm;
    }
    else {
      AssertMsg(src != X86_ESP,"Can't use dword displacement from sp");
      *pbuf++ = MakeModRM(MOD_DWORD_DISPLACEMENT,rd,src);
        *(unaligned int *)pbuf=imm;
        pbuf+=4;
    }
}

void RTLGenerator::AddOffsetX86(int reg,int offset) {
    if (X86FitsSimm8(offset)) {
       *pbuf++ = I_ADD_MI8;
       *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI8_RVAL,reg);
       *pbuf++ = (unsigned char)offset;
    }
    else {
       *pbuf++ = I_ADD_MI32;
       *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI32_RVAL,reg);
       *(unaligned int *)pbuf = offset;
       pbuf+=4;
    }
}

// arguments have been pushed right-to-left; call target and clean up stack
void RTLGenerator::EmitCallIX86(CallInstruction *callInstruction) {
    //  load native entry point from script function into eax
    int epOffset=offsetof(Js::JavascriptFunction,NativeEntryPoint);
    *pbuf++ = I_LD;
    SetMovBaseOps(X86_EAX,callInstruction->mrs1,epOffset,SCALE_1);
    // push number of arguments (including 'this')
    PushImmediateX86(callInstruction->nArgs);
    // push function wrapper
    *pbuf++ = I_PUSH_REG|(unsigned char)callInstruction->mrs1;
    // call native entry point
    *pbuf++ = I_ICALL;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ICALL_RVAL,X86_EAX);
    // move the return value if necessary
    int reg=callInstruction->mrd;
    if (reg!=X86_EAX) {
        // move return value to destination register
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,reg,X86_EAX);
    }
    // clean up (FIX: need way of not doing this if stdcall;
    // for example, could determine at call site whether stdcall)
    // CHECK: make sure we are popping any extra args
    int offset=(callInstruction->nArgs+2)*sizeof(int);
    AddOffsetX86(X86_ESP,offset);
}

Js::Atom NewScObjectX86MemContext(MemoryContext* memContext) {
    Js::DynamicObject* dynamicObject = RecyclerNew(memContext->GetRecycler(),Js::DynamicObject,NULL,memContext);
    //Js::ScriptContext* scriptContext=Js::ScriptContext::Info(memContext);
    //scriptContext->IncrObjectCount();
    Js::Atom aNewInstance = Js::RecyclableObject::ToAtom(dynamicObject);
    return aNewInstance;
}

Js::Atom NewScObjectX86(Js::Atom function) {
    //
    // Get the "constructor function" to use for the new instance's internal [[Prototype]] property.
    //

    Js::JavascriptFunction* constructorFunction = null;
    if (Js::JavascriptFunction::Is(function)) {
        constructorFunction = Js::JavascriptFunction::FromAtom(function);
    }

    //
    // Allocate a new, empty DynamicObject instance:
    // - For non-JavascriptFunction objects, this is the only time that the object's internal
    //   [[Prototype]] property may be set.  After this point, it becomes read-only.
    //

    MemoryContext* memContext=constructorFunction->GetMemoryContext();
    Js::DynamicObject* dynamicObject = RecyclerNew(memContext->GetRecycler(),Js::DynamicObject,constructorFunction,memContext);
    //Js::ScriptContext* scriptContext=Js::ScriptContext::Info(memContext);
    //scriptContext->IncrObjectCount();
    Js::Atom aNewInstance = Js::RecyclableObject::ToAtom(dynamicObject);
    return aNewInstance;
}

void RTLGenerator::EmitNewScObjectX86(CallInstruction *callInstruction) {
    unsigned char *branch1;
    unsigned char *branch2;
    unsigned char *branch3;
    void *fn;

    int destReg=callInstruction->mrd;
    RegisterDescription *registerDescription=registerDescriptions[callInstruction->rs1];
    if ((registerDescription->GetRegisterType()==RegisterTypeObjectConstant)&&
        (registerDescription->GetConstDefOp()==Js::OpCode::LdC_A_Null)) {
            fn=NewScObjectX86MemContext;
            // just create new object (no constructor)
            PushImmediateX86((int)memContext);
            FinishStdCall(destReg,fn);
    }
    else {
        fn=NewScObjectX86;
        int fnReg=callInstruction->mrs1;
        // if (func!=NULL) {
        *pbuf++ = I_CMP_I32;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I32_RVAL,fnReg);
        *(int *)pbuf=0;
        pbuf+=4;

        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_EQ;
        *(unaligned int *)pbuf=0;
        branch1=(unsigned char *)pbuf;
        pbuf+=4;

        int offset= -1;
        if (fnReg==destReg) {
            // create frame slot to stash mrs1
            curLocalSlot-=4;
            offset=curLocalSlot;
            // stash fnReg
            *pbuf++ = I_ST;
            SetMovBaseOps(fnReg,X86_EBP,offset,SCALE_1);
        }
        //  push prototype as argument
        *pbuf++ = I_PUSH_REG|(unsigned char)fnReg;
        FinishStdCall(destReg,fn);
        // destReg has new object
        //*pbuf++ = I_PUSH_REG|(unsigned char)destReg;
        // place 'this' pointer in arg 0 slot
        // mov ecx,esp
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,X86_ECX,X86_ESP);

        EmitStoreBaseX86(destReg,X86_ECX,0);
        if (destReg==fnReg) {
            *pbuf++ = I_LD;
            //  load edx from stashed fnReg
            SetMovBaseOps(X86_EDX,X86_EBP,offset,SCALE_1);
            callInstruction->mrs1=X86_EDX;
        }
        callInstruction->mrd=X86_EAX;
        EmitCallIX86(callInstruction);

        // FIX: TODO: pin this to this code object
        void *undefinedObject=Js::ScriptContext::Info(memContext)->GetUndefinedValueA();
        *pbuf++ = I_CMP_I32;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I32_RVAL,X86_EAX);
        *(unaligned int *)pbuf=(int)undefinedObject;
        pbuf+=4;
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_EQ;
        *(unaligned int *)pbuf=0;
        branch2=(unsigned char *)pbuf;
        pbuf+=4;
        // mov destReg,eax
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,destReg,X86_EAX);
        // branch target that skips register move
        *(int*)branch2=(int)(pbuf-(4+branch2));
        *pbuf++ = I_JMP_PCREL;
        *(unaligned int *)pbuf=0;
        branch3=pbuf;
        pbuf+=4;
        // branch target for big else
        *(int*)branch1=(unsigned int)(pbuf-(4+branch1));
        PushImmediateX86(0);
        FinishStdCall(destReg,fn);
        // exit from this big gizmo
        *(int*)branch3=(unsigned int)(pbuf-(4+branch3));
    }
}

void RTLGenerator::MoveToArgSlot(int sourceReg,int spOffset) {
    *pbuf++ = I_ST;
    *pbuf++ = MakeModRM(MOD_DWORD_DISPLACEMENT,sourceReg,RM_IS_SIB);
    *pbuf++ = MakeSIB(SCALE_1,INDEX_IS_NONE,X86_ESP);
    *(int *)pbuf = spOffset;
    pbuf+=4;
}

void RTLGenerator::MoveIntToArgSlot(int val,int spOffset) {
    *pbuf++ = I_MOVDW;
    *pbuf++ = MakeModRM(MOD_DWORD_DISPLACEMENT,I_MOVDW_RVAL,RM_IS_SIB);
    *pbuf++ = MakeSIB(SCALE_1,INDEX_IS_NONE,X86_ESP);
    *(int *)pbuf = spOffset;
    pbuf+=4;
    *(int *)pbuf = val;
    pbuf+=4;
}

void RTLGenerator::ArgOutConstantAtomX86(RegisterDescription *registerDescription,int scratchReg,int spOffset) {
    void *fn;
    switch (registerDescription->GetConstDefOp()) {
        case Js::OpCode::LdRoot:
            PushImmediateX86((int)memContext);
            fn=Js::DynamicObject::OP_LdRoot;
            FinishStdCall(scratchReg,fn);
            MoveToArgSlot(scratchReg,spOffset);
            break;
        case Js::OpCode::LdUndef:
            PushImmediateX86((int)memContext);
            fn=Js::DynamicObject::OP_LdUndef;
            FinishStdCall(scratchReg,fn);
            MoveToArgSlot(scratchReg,spOffset);
            break;
        case Js::OpCode::LdTrue:
            PushImmediateX86((int)memContext);
            fn=Js::JavascriptBoolean::OP_LdTrue;
            FinishStdCall(scratchReg,fn);
            MoveToArgSlot(scratchReg,spOffset);
            break;
        case Js::OpCode::LdFalse:
            PushImmediateX86((int)memContext);
            fn=Js::JavascriptBoolean::OP_LdFalse;
            FinishStdCall(scratchReg,fn);
            MoveToArgSlot(scratchReg,spOffset);
            break;
        case Js::OpCode::LdC_A_Null:
            MoveIntToArgSlot(0,spOffset);
            break;
        case Js::OpCode::LdC_A_I4_0:
            MoveIntToArgSlot((int)Js::JavascriptNumber::ToAtom((int)0,memContext),spOffset);
            break;
        case Js::OpCode::LdC_A_I4_1:
            MoveIntToArgSlot((int)Js::JavascriptNumber::ToAtom((int)1,memContext),spOffset);
            break;
        case Js::OpCode::LdC_A_I4:
            MoveIntToArgSlot((int)Js::JavascriptNumber::ToAtom((int)registerDescription->GetIntConst(),memContext),spOffset);
            break;
        case Js::OpCode::LdC_A_R8:
        case Js::OpCode::LdC_A_R8_0: {
            // FIX: TODO: GC must not collect until code collected so put on list w/ code
            Js::Atom a=Js::JavascriptNumber::ToAtom(registerDescription->GetDoubleConst(),memContext);
            *pbuf++ = I_MOVDW_REG|(unsigned char)scratchReg;
            *(unaligned Js::Atom *)pbuf=a;
            pbuf+=4;
            MoveToArgSlot(scratchReg,spOffset);
            break;
        }
    }
}

void RTLGenerator::ArgOutOperandX86(int operand,int machineRegOperand,int scratchReg,int spOffset) {
    RegisterDescription *operandDescription=registerDescriptions[operand];
    RegisterType regType=operandDescription->GetRegisterType();
    if ((regType==RegisterTypeIntConstant)||(regType==RegisterTypeDoubleConstant)||(regType==RegisterTypeObjectConstant)) {
        ArgOutConstantAtomX86(operandDescription,scratchReg,spOffset);
    }
    else MoveToArgSlot(machineRegOperand,spOffset);
}

// assumes arg out instructions have been ordered right to left (high to low index)
void RTLGenerator::EmitArgOutX86(ArgInstruction *argInstruction) {
    AssertMsg(argInstruction->op==Js::OpCode::TempArgOut_A,"Only output args using TempArgOut_A opcode");
    // TODO: have load operand only move the register if necessary
    ArgOutOperandX86(argInstruction->rs1,argInstruction->mrs1,X86_EDX,argInstruction->argIndex*4);
/*
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_ECX,X86_ESP);

    EmitStoreBaseX86(X86_EDX,X86_ECX,argInstruction->argIndex*4);
    */
}

void RTLGenerator::FinishStdCall(int destReg,void *fn) {
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->relocType=RelocTypeCallPcrel;
    *pbuf++ = I_CALL_PCREL;
    reloc->consumerOffset=(unsigned int)(pbuf-codeBuffer);
    // this will be updated by a relocation
    *(unaligned int*)(pbuf)=(unsigned int)fn;
    //printf("trying to call helper at %x\n",fn);
    pbuf+=4;  // pc-relative offset
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);
    if (destReg!=X86_EAX) {
        // move return value to destination register
        *pbuf++ = I_MOV_RR;
        *pbuf++ = MakeModRM(MOD_IS_REG,destReg,X86_EAX);
    }
}

void RTLGenerator::PushImmediateX86(int imm) {
    if (X86FitsSimm8(imm)) {
        *pbuf++ = I_PUSH_MI8;
        *pbuf++ = (char)imm;
    }
    else {
        *pbuf++ = I_PUSH_MI32;
        *(unaligned int *)pbuf = imm;
        pbuf+=4;
    }
}

void RTLGenerator::EmitLoadPropertyX86(IntConstInstruction *instruction) {
    void *fn;
    if (instruction->op==Js::OpCode::LdFld) {
        unsigned char *branchToSlowPath1;
        unsigned char *branchToSlowPath2;
        unsigned char *branchAroundSlowPath;
        // four arguments *type,*slotIndex,instance, fieldId
        LoadPatch *loadPatch=MakeStruct(alloc,LoadPatch);
        loadPatch->type=(void*)0xfafabead;  // this can not be a type pointer
        loadPatch->slotIndex=(-1);
        // test mrs1,0x1  (check for tag bit)
        *pbuf++ = I_TEST_MI8;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_TEST_MI8_RVAL,instruction->mrs1);
        *pbuf++ = 3;
        // branch
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NZ;
        branchToSlowPath1=pbuf;
        pbuf+=4;
        // load type into edx
        EmitLoadBaseX86(X86_EDX,instruction->mrs1,offsetof(Js::RecyclableObject, type));
        *pbuf++ = I_CMP_RM;
        SetMovBaseOps(X86_EDX,X86_REG_NONE,(int)&(loadPatch->type),SCALE_1);
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NE;
        branchToSlowPath2=pbuf;
        pbuf+=4;
        // fast path
        // load slot pointer into edx
        EmitLoadBaseX86(X86_EDX,instruction->mrs1,offsetof(Js::DynamicObject,m_pSlots));
        *pbuf++ = I_LD;
        // load cached slot index into ecx
        SetMovBaseOps(X86_ECX,X86_REG_NONE,(int)&(loadPatch->slotIndex),SCALE_1);
        // mov mrd,[edx+4*ecx]
        *pbuf++ = I_LD;
        *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,instruction->mrd,RM_IS_SIB);
        *pbuf++ = MakeSIB(SCALE_4,X86_ECX,X86_EDX);
        // jump to exit
        *pbuf++ = I_JMP_PCREL; 
        branchAroundSlowPath=pbuf;
        pbuf+=4;
        // slow path
        *(int*)branchToSlowPath1=(unsigned int)(pbuf-(4+branchToSlowPath1));
        *(int*)branchToSlowPath2=(unsigned int)(pbuf-(4+branchToSlowPath2));

        PushImmediateX86((int)memContext);
        PushImmediateX86(instruction->immInt);
        PushOperandX86(instruction->rs1,instruction->mrs1);
        PushImmediateX86((int)&(loadPatch->slotIndex));
        PushImmediateX86((int)&(loadPatch->type));
        fn=Js::DynamicObject::PatchGetValue;
        FinishStdCall(instruction->mrd,fn);
        // exit
        *(int*)branchAroundSlowPath=(unsigned int)(pbuf-(4+branchAroundSlowPath));
    }
    else {
        if (instruction->op==Js::OpCode::NewScFunc) {
            fn=Js::JavascriptFunction::OP_NewScFunc;
        }
        else if (instruction->op==Js::OpCode::ScopedLdFld) {
            fn=Js::JavascriptOperators::GetPropertyScoped;
            PushImmediateX86((int)memContext);
        }
        else {
            fn=Js::FixedSizeArray::OP_LdElemC;
        }
        // push the operands right to left
        PushImmediateX86(instruction->immInt);
        PushOperandX86(instruction->rs1,instruction->mrs1);
        // call the helper
        FinishStdCall(instruction->mrd,fn);
    }
}

void RTLGenerator::EmitStorePropertyX86(IntConstInstruction *instruction) {
    void *fn;
#define INLINE_CACHE
#ifdef INLINE_CACHE
    if (instruction->op==Js::OpCode::StFld) {
        unsigned char *branchToSlowPath1;
        unsigned char *branchToSlowPath2;
        unsigned char *branchAroundSlowPath;
        // four arguments *type,*slotIndex,instance, fieldId
        LoadPatch *loadPatch=MakeStruct(alloc,LoadPatch);
        loadPatch->type=(void*)0xfafabeeb;  // this can not be a type pointer
        loadPatch->slotIndex=(-1);
        // check that lhs is an object
        // test mrs1,0x1  (check for tag bit)
        *pbuf++ = I_TEST_MI8;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_TEST_MI8_RVAL,instruction->mrs1);
        *pbuf++ = 3;
        // branch
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NZ;
        branchToSlowPath1=pbuf;
        pbuf+=4;
        // load type into edx
        EmitLoadBaseX86(X86_EDX,instruction->mrs1,offsetof(Js::RecyclableObject, type));
        *pbuf++ = I_CMP_RM;
        SetMovBaseOps(X86_EDX,X86_REG_NONE,(int)&(loadPatch->type),SCALE_1);
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NE;
        branchToSlowPath2=pbuf;
        pbuf+=4;
        // fast path
        // load slot pointer into edx
        EmitLoadBaseX86(X86_EDX,instruction->mrs1,offsetof(Js::DynamicObject,m_pSlots));
        // mov eax, mrs2 
        LoadOperandX86(instruction->rs2,instruction->mrs2,X86_EAX);
        *pbuf++ = I_LD;
        // load cached slot index into ecx
        SetMovBaseOps(X86_ECX,X86_REG_NONE,(int)&(loadPatch->slotIndex),SCALE_1);
        // mov [edx+4*ecx],mrd
        *pbuf++ = I_ST;
        *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,X86_EAX,RM_IS_SIB);
        *pbuf++ = MakeSIB(SCALE_4,X86_ECX,X86_EDX);
        // jump to exit
        *pbuf++ = I_JMP_PCREL; 
        branchAroundSlowPath=pbuf;
        pbuf+=4;
        // slow path
        *(int*)branchToSlowPath1=(unsigned int)(pbuf-(4+branchToSlowPath1));
        *(int*)branchToSlowPath2=(unsigned int)(pbuf-(4+branchToSlowPath2));
        PushOperandX86(instruction->rs2,instruction->mrs2);
        PushImmediateX86(instruction->immInt);
        PushOperandX86(instruction->rs1,instruction->mrs1);
        PushImmediateX86((int)&(loadPatch->slotIndex));
        PushImmediateX86((int)&(loadPatch->type));
        fn=Js::DynamicObject::PatchPutValue;
        FinishStdCall(X86_EAX,fn);
        // exit
        *(int*)branchAroundSlowPath=(unsigned int)(pbuf-(4+branchAroundSlowPath));
    }
    else {
        if (instruction->op==Js::OpCode::ScopedStFld) {
            fn=Js::JavascriptOperators::SetPropertyScoped;
            PushImmediateX86((int)memContext);
        }
        else if (instruction->op==Js::OpCode::InitFld) {
            fn=Js::JavascriptOperators::InitProperty;
        }
        else {
            fn=Js::FixedSizeArray::OP_StElemC;
        }
        // push the operands right to left
        PushOperandX86(instruction->rs2,instruction->mrs2);
        PushImmediateX86(instruction->immInt);
        PushOperandX86(instruction->rs1,instruction->mrs1);
        // call the helper
        FinishStdCall(X86_EAX,fn);
    }
#else
        // push the operands right to left
        PushOperandX86(instruction->rs2,instruction->mrs2);
        PushImmediateX86(instruction->immInt);
        PushOperandX86(instruction->rs1,instruction->mrs1);
        // call the helper
        if (instruction->op==Js::OpCode::StFld) {
            fn=Js::JavascriptOperators::SetProperty;
        }
        else fn=Js::FixedSizeArray::OP_StElemC;
        FinishStdCall(X86_EAX,fn);
#endif
}


// REVIEW: check lhs tag bit as in 3["toString"]
void RTLGenerator::EmitLoadElementI(Instruction *instruction) {
    PushImmediateX86((int)memContext);
    // push the operands right to left
    PushOperandX86(instruction->rs2,instruction->mrs2);
    PushOperandX86(instruction->rs1,instruction->mrs1);
    // call the helper
    void *fn=Js::JavascriptOperators::GetElementI;
    FinishStdCall(instruction->mrd,fn);
}

// TODO: inline caching
void RTLGenerator::EmitStoreElementI(StElemInstruction *instruction) {
    // push the operands right to left
    PushOperandX86(instruction->val,instruction->mval);
    PushOperandX86(instruction->rs2,instruction->mrs2);
    PushOperandX86(instruction->rs1,instruction->mrs1);
    // call the helper
    void *fn=Js::JavascriptOperators::SetElementI;
    FinishStdCall(X86_EAX,fn);
}

int RTLGenerator::AssignFrameSlotX86(RegisterDescription *registerDescription) {
    int offset=registerDescription->GetFramePointerOffset();
    if (offset==RegisterDescription::NoFramePointerOffset) {
        curLocalSlot-=4;
        offset=curLocalSlot;
        registerDescription->SetFramePointerOffset(offset);
    }
    return offset;
}

void RTLGenerator::EmitStoreBaseX86(int machineReg,int base,int offset) {
    *pbuf++ = I_ST;
    SetMovBaseOps(machineReg,base,offset,SCALE_1);
}

void RTLGenerator::EmitLoadBaseX86(int machineReg,int base,int offset) {
    *pbuf++ = I_LD;
    SetMovBaseOps(machineReg,base,offset,SCALE_1);
}

void RTLGenerator::EmitSpillX86(Instruction *instruction){ 
   int machineReg=instruction->mrs1;
   int pseudoReg=instruction->rs2;
   RegisterDescription *registerDescription=registerDescriptions[pseudoReg];
   int offset=AssignFrameSlotX86(registerDescription);
   EmitStoreBaseX86(machineReg,X86_EBP,offset);
}

void RTLGenerator::EmitReloadX86(Instruction *instruction) {
   int machineReg=instruction->mrd;
   int pseudoReg=instruction->rs1;
   RegisterDescription *registerDescription=registerDescriptions[pseudoReg];
   int offset=registerDescription->GetFramePointerOffset();
   AssertMsg(offset!=RegisterDescription::NoFramePointerOffset,"Reload from unassigned stack slot");
   EmitLoadBaseX86(machineReg,X86_EBP,offset);
}

void RTLGenerator::EmitArgInX86(ArgInstruction *argInstruction) {
   unsigned char *branchToLoadNull;
   unsigned char *branchToExit;
   int machineReg=argInstruction->mrd;
   int pseudoReg=argInstruction->rd;
   RegisterDescription *registerDescription=registerDescriptions[pseudoReg];
   int offset=registerDescription->GetFramePointerOffset();
   AssertMsg(offset!=RegisterDescription::NoFramePointerOffset,"Reload from unassigned stack slot");
   AssertMsg(argCountCurrentFunction<128,"TODO: use wider immediate if necessary for arg count");
   EmitLoadBaseX86(X86_EAX,X86_EBP,12); // 12(ebp) is argCount argument
   *pbuf++ = I_CMP_I8_EAX;
   *pbuf++ = (unsigned char)argInstruction->argIndex;
   *pbuf++ = I_JCC1;
   *pbuf++ = I_ICOND_LE;
   branchToLoadNull=pbuf;
   pbuf+=4;
   EmitLoadBaseX86(machineReg,X86_EBP,offset);
   *pbuf++ = I_JMP_PCREL;
   branchToExit=pbuf;
   pbuf+=4;
   *(unaligned int *)branchToLoadNull=(int)(pbuf-(branchToLoadNull+4));
   *pbuf++ = I_XOR_MR;
   *pbuf++ = MakeModRM(MOD_IS_REG,machineReg,machineReg);
   *(unaligned int *)branchToExit=(int)(pbuf-(branchToExit+4));
}

void RTLGenerator::EmitExchangeLocationsX86(Instruction *instruction) {
   int machineReg=instruction->mrd;
   int toPseudoReg=instruction->rs1;
   int fromPseudoReg=instruction->rs2;
   // spill
   RegisterDescription *registerDescription=registerDescriptions[toPseudoReg];
   int offset=AssignFrameSlotX86(registerDescription);
   EmitStoreBaseX86(machineReg,X86_EBP,offset);
   // reload
   registerDescription=registerDescriptions[fromPseudoReg];
   offset=registerDescription->GetFramePointerOffset();
   AssertMsg(offset!=RegisterDescription::NoFramePointerOffset,"Reload from unassigned stack slot");
   EmitLoadBaseX86(machineReg,X86_EBP,offset);
}

void RTLGenerator::EmitLdStr(LdStrInstruction *instruction) {
    *pbuf++ = I_MOVDW_REG|(unsigned char)instruction->mrd;
    *(int *)pbuf=(int)instruction->str;
    pbuf+=4;
}

void RTLGenerator::EmitNewRegEx(RegexInstruction *instruction) {
    *pbuf++ = I_MOVDW_REG|(unsigned char)instruction->mrd;
    *(int *)pbuf=(int)(Js::JavascriptRegularExpression::OP_NewRegEx((Js::Atom)instruction->regularExpression,memContext));
    pbuf+=4;
}

void RTLGenerator::EmitLdEnv(Instruction *instruction) {
   EmitLoadBaseX86(instruction->mrd,X86_EBP,8); // 8(ebp) is func argument
   EmitLoadBaseX86(instruction->mrd,instruction->mrd,offsetof(Js::JavascriptFunction,Environment));
}

void RTLGenerator::EmitPrologueX86() {
    // push ebp
    *pbuf++ = I_PUSH_REG|X86_EBP;

   // mov ebp,esp
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EBP,X86_ESP);

    // add esp,slotsDecr
    *pbuf++ = I_ADD_MI32;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI32_RVAL,X86_ESP);
    *(unaligned int *)pbuf=0;
    NativeReloc *reloc=MakeStruct(alloc,NativeReloc);
    reloc->relocType=RelocTypeLocalSlotsOffset;
    reloc->consumerOffset=(unsigned int)(pbuf-codeBuffer);
    pbuf+=4;
    ListFn<NativeReloc*>::Add(nativeRelocs,reloc,alloc);

    // push callee-saved regs 
    *pbuf++ = I_PUSH_REG|X86_EBX;
    *pbuf++ = I_PUSH_REG|X86_ESI;
    *pbuf++ = I_PUSH_REG|X86_EDI;
}

void RTLGenerator::EmitEpilogueX86(int slotOffset) {
    // pop callee-saved
    *pbuf++ = I_POP_REG|X86_EDI;
    *pbuf++ = I_POP_REG|X86_ESI;
    *pbuf++ = I_POP_REG|X86_EBX;

    // add esp,-slotsDecr
    *pbuf++ = I_ADD_MI32;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ADD_MI32_RVAL,X86_ESP);
    *(unaligned int *)pbuf= -slotOffset;
    pbuf+=4;

   // mov esp,ebp
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_ESP,X86_EBP);

    // pop ebp
    *pbuf++ = I_POP_REG|X86_EBP;

    // return
    *pbuf++ = I_RET;
}

void RTLGenerator::EmitTraceBBX86(BasicBlock *block) {
    if (instructionStats!=NULL) {
        BlockStats *blockStats=MakeStruct(alloc,BlockStats);
        blockStats->block=block;
        blockStats->execCount=0;
        // increment the counter
        *pbuf++ = I_INC_RM;
        *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,I_INC_RM_RVAL,RM_IS_DIRECT);
        *(void **)pbuf = &(blockStats->execCount);
        pbuf+=4;
        instructionStats->AddTracedBlock(blockStats);
    }
}

// Generates buffer of X86 code

void RTLGenerator::EmitFunctionX86(ControlFlowGraph *cfg,int nRegs) {
    //pendingCalls=ListFn<CallDescription*>::MakeListHead(alloc);
    firstLocalSlot = 0;
    curLocalSlot=firstLocalSlot;
    nativeRelocs=ListFn<NativeReloc*>::MakeListHead(alloc);
    // allocate a big buffer and then copy and trim at end
    codeBufferSize=(40*instructionCount)+200;
    codeBuffer=(unsigned char *)calloc(codeBufferSize,1);
    pbuf=codeBuffer;
    ListFn<BasicBlock*>::Add(cfg->GetBlocks(),cfg->GetExit(),alloc);
    EmitPrologueX86();
    for (List<BasicBlock*> *blockEntry=cfg->GetBlocks()->next;!(blockEntry->isHead);blockEntry=blockEntry->next) {
        BasicBlock *block=blockEntry->data;
        block->SetNativeStartOffset((unsigned int)(pbuf-codeBuffer));
        if (traceBB) {
            EmitTraceBBX86(block);
        }
        for (List<Instruction*> *instrEntry=block->GetInstructions()->next;!(instrEntry->isHead);instrEntry=instrEntry->next) {
            Instruction *instruction=instrEntry->data;
            Js::OpCode op=instruction->op;
            switch (op) {
            case Js::OpCode::FunctionExit: {
                EmitEpilogueX86(curLocalSlot);
                break;
            }
            case Js::OpCode::Nop: {
                break;
            }
            case Js::OpCode::Ret: {
                EmitUnconditionalBranchX86((BranchInstruction *)instruction);
                break;
            }
            case Js::OpCode::BrEq_A: 
            case Js::OpCode::BrNeq_A: 
            case Js::OpCode::BrSrEq_A: 
            case Js::OpCode::BrSrNeq_A: 
            case Js::OpCode::BrGt_A: 
            case Js::OpCode::BrLt_A: 
            case Js::OpCode::BrGe_A: 
            case Js::OpCode::BrLe_A: 
                EmitConditionalBranchX86((BranchInstruction*)instruction,true);
                break;
            case Js::OpCode::Add_A:
            case Js::OpCode::Div_A:
            case Js::OpCode::Mul_A:
            case Js::OpCode::Rem_A:
            case Js::OpCode::Sub_A:
            case Js::OpCode::And_A:
            case Js::OpCode::Or_A:
            case Js::OpCode::Xor_A:
            case Js::OpCode::Shl_A:
            case Js::OpCode::Shr_A:
            case Js::OpCode::ShrU_A:
            case Js::OpCode::CmEq_A:
            case Js::OpCode::CmGt_A:
            case Js::OpCode::CmGe_A:
            case Js::OpCode::CmLt_A:
            case Js::OpCode::CmLe_A:
            case Js::OpCode::CmNeq_A:
            case Js::OpCode::IsInst:
            case Js::OpCode::LdFrameDisplay:
                EmitRegisterOperation3X86(instruction);
                break;
            case Js::OpCode::CallI:
                EmitCallIX86((CallInstruction*)instruction);
                break;
            case Js::OpCode::NewScObject:
            case Js::OpCode::NewScObjectSimple:
                EmitNewScObjectX86((CallInstruction*)instruction);
                break;
            case Js::OpCode::Ld_A:
                LoadOperandX86(instruction->rs1,instruction->mrs1,instruction->mrd);
                break;
            case Js::OpCode::Neg_A:
            case Js::OpCode::Not_A:
            case Js::OpCode::LdLen_A:
            case Js::OpCode::NewArray:
            case Js::OpCode::NewScArray:
            case Js::OpCode::GetEnumerator:
            case Js::OpCode::GetCurrent:
            case Js::OpCode::LdHandlerScope:
                EmitRegisterOperation2X86(instruction);
                break;
            case Js::OpCode::Br:
                EmitUnconditionalBranchX86((BranchInstruction *)instruction);
                break;
            case Js::OpCode::LdRoot:
                EmitLoadRootX86(registerDescriptions[instruction->rd],instruction->mrd);
                break;
            case Js::OpCode::LdUndef:
                if (instruction->mrd==ReturnRegister) {
                    EmitLoadUndefX86(ReturnRegister);
                }
                break;
            case Js::OpCode::LdTrue:
            case Js::OpCode::LdC_A_Null:
            case Js::OpCode::LdC_A_I4_0:
            case Js::OpCode::LdFalse:
            case Js::OpCode::LdC_A_I4_1:
            case Js::OpCode::LdC_A_I4:
            case Js::OpCode::LdC_A_R8:
            case Js::OpCode::LdC_A_R8_0:
                // these are emitted on use as an operand
                break;
            case Js::OpCode::BrFalse_A:
            case Js::OpCode::BrTrue_A:
            case Js::OpCode::BrOnEmpty:
                EmitConditionalBranchX86((BranchInstruction*)instruction,false);
                break;
            case Js::OpCode::BrOnHasProperty:
            case Js::OpCode::BrOnNoProperty:
                EmitPropertyBranchX86((BranchImmInstruction*)instruction);
                break;
            case Js::OpCode::ArgIn_A:
                EmitArgInX86((ArgInstruction*)instruction);
                break;
            case Js::OpCode::TempArgOut_A:
                EmitArgOutX86((ArgInstruction*)instruction);
                break;
            case Js::OpCode::ArgOut_A:
                // Using TempArgOut instead
                //EmitArgOutX86((ArgInstruction*)instruction);
                break;
            case Js::OpCode::LdFld:
                EmitLoadPropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::ScopedLdFld:
                EmitLoadPropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::NewScFunc:
                EmitLoadPropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::StFld:
            case Js::OpCode::InitFld:
                EmitStorePropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::LdElemC:
                EmitLoadPropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::StElemC:
            case Js::OpCode::ScopedStFld:
                EmitStorePropertyX86((IntConstInstruction*)instruction);
                break;
            case Js::OpCode::LdElemI_A:
                EmitLoadElementI(instruction);
                break;
            case Js::OpCode::StElemI_A:
                EmitStoreElementI((StElemInstruction*)instruction);
                break;
            case Js::OpCode::LdStr: {
                EmitLdStr((LdStrInstruction *)instruction);
                break;
            }
            case Js::OpCode::NewRegEx: {
                EmitNewRegEx((RegexInstruction *)instruction);
                break;
            }
            case Js::OpCode::StartCall: {
                // esp -= 4*nActualArgs
                IntConstInstruction *startCallInstruction=(IntConstInstruction*)instruction;
                *pbuf++ = I_SUB_MI32;
                *pbuf++ = MakeModRM(MOD_IS_REG,I_SUB_MI32_RVAL,X86_ESP);
                *(unaligned int *)pbuf=4*(startCallInstruction->immInt);
                pbuf+=4;
                break;
            }
            case Js::OpCode::LdEnv:
                EmitLdEnv(instruction);
                break;
            case Js::OpCode::Spill: {
                EmitSpillX86(instruction);
                break;
            }
            case Js::OpCode::Reload: {
                EmitReloadX86(instruction);
                break;
            }
            case Js::OpCode::ExchangeLocations:
                EmitExchangeLocationsX86(instruction);
                break;
#if DBG_DUMP
            case Js::OpCode::PosInSrc: {
                break;
            }
#endif
            default: {
                AssertMsg(false,"Unimplemented opcode");
                break;
            }
            }
            AssertMsg((pbuf-codeBuffer)<(int)codeBufferSize,"code buffer overflow");
        }

    }
    unsigned int permaBufferSize=(unsigned int)(pbuf-codeBuffer);
    // round up to DWORD boundary
    permaBufferSize=permaBufferSize+(4-permaBufferSize&0x3);
    unsigned char *permaBuffer=(unsigned char *)VirtualAlloc(NULL,permaBufferSize,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    memcpy_s(permaBuffer,permaBufferSize,codeBuffer,pbuf-codeBuffer);
    free(codeBuffer);
    codeBuffer=permaBuffer;
    ApplyNativeRelocsX86();
    if (!silent)
       print_raw_as_insns(codeBuffer,permaBufferSize,(unsigned int)codeBuffer);
}
