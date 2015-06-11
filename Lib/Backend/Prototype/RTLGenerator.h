/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class RTLGenerator {
    MemoryContext* memContext;
    ArenaAllocator *alloc;
    ControlFlowGraph *cfg;
    Instruction **offsetToInstruction;
    RegisterDescription **registerDescriptions;
    List<Reloc*> *relocs;
    BasicBlock *currentBasicBlock;
    Js::ByteCodeReader m_reader;
    Js::FunctionBody *fn;
    unsigned int functionStartOffset;
    int instructionCount;
    // for emitting native code
    unsigned char *codeBuffer;
    unsigned int codeBufferSize;
    unaligned unsigned char *pbuf;
    List<NativeReloc*> *nativeRelocs;
    int firstLocalSlot;     
    int curLocalSlot;
    int argCountCurrentFunction;
    bool silent;
    int nRegs;
public:
    RTLGenerator(MemoryContext* memContext,ArenaAllocator *alloc,bool silent): memContext(memContext),
        alloc(alloc), silent(silent), instructionCount(0) {
    }

    inline MemoryContext* GetMemoryContext() { return memContext; }

    RegisterDescription *GetRegisterDescription(Js::RegSlot r)
    {
        return registerDescriptions[r];
    }

    // FIX: TODO: unify this constant with ByteCodeGenerator usage
    static const int ReturnRegister=0;
    void BuildCFG();
    Instruction *ConditionalBranch(Js::OpCode op);
    Instruction *ConditionalBranchReg1(Js::OpCode op);
    Instruction *ConditionalBranchProperty(Js::OpCode op);
    Instruction *Branch(Js::OpCode op);
    Instruction *ReturnInstruction(Js::OpCode op);
    Instruction *StartCallInstruction(Js::OpCode op);
    Instruction *RegisterOperation3(Js::OpCode op);
    Instruction *RegisterOperation2(Js::OpCode op);
    Instruction *LoadConstant(Js::OpCode op);
    Instruction *LoadRoot(Js::OpCode op);
    Instruction *LoadEnv(Js::OpCode op);
    Instruction *LoadConstantI4(Js::OpCode op);
    Instruction *LoadConstantR8(Js::OpCode op);
    Instruction *LoadAux(Js::OpCode op);
    Instruction *ArgIn(Js::OpCode op);
    Instruction *ArgOut(Js::OpCode op);
    Instruction *PropertyAccess(Js::OpCode op);
    Instruction *ElementAccess(Js::OpCode op);
    Instruction *CallDirect(Js::OpCode op);
    Instruction *CallI(Js::OpCode op);
    Instruction *CallIM(Js::OpCode op);
    Instruction *LdProperty(Js::OpCode op);
    Instruction *StProperty(Js::OpCode op);
    Instruction *LdElementDirect(Js::OpCode op);
    Instruction *StElementDirect(Js::OpCode op);
    Instruction *LdElement(Js::OpCode op);
    Instruction *StElement(Js::OpCode op);
    Instruction *NewRegEx(Js::OpCode op);

    void PrintDags(ControlFlowGraph *cfg);
    void ApplyRelocs();
    void AddConditionalBranchInstruction(BranchInstruction *instruction,unsigned int targetOffset);
    void AddBranchInstruction(BranchInstruction *instruction,unsigned int targetOffset   );
    void CloseBasicBlock(bool canFallThrough,unsigned int targetOffset);
    void CloseBasicBlock(bool canFallThrough,BasicBlock *targetBlock);
    void NewBasicBlock(bool canFallThrough);

    // TODO: remove nRegs arg in favor of using fn->Locals
    ControlFlowGraph *TranslateFunction(Js::FunctionBody *fn,int nRegs);
    void MoveIntToArgSlot(int val,int spOffset);
    void MoveToArgSlot(int sourceReg,int spOffset);
    void ArgOutConstantAtomX86(RegisterDescription *registerDescription,int scratchReg,int spOffset);
    void ArgOutOperandX86(int operand,int machineRegOperand,int destReg,int spOffset);
    void ApplyNativeRelocsX86();
    void EmitTraceBBX86(BasicBlock *block);
    void EmitNewScObjectX86(CallInstruction *callInstruction);
    void EmitLdStr(LdStrInstruction *instruction);
    void EmitNewRegEx(RegexInstruction *instruction);
    void EmitLdEnv(Instruction *instruction);
    void EmitLoadRootX86(RegisterDescription *registerDescription,int reg);
    void EmitLoadUndefX86(int reg);
    void EmitCallIX86(CallInstruction *callInstruction);
    void PushImmediateX86(int imm);
    void EmitFunctionUsingDagsX86(ControlFlowGraph *cfg,int nRegs);
    void EmitFunctionX86(ControlFlowGraph *cfg,int nRegs);
    void PushOperandX86(int operand,int machineRegOperand);
    void LoadOperandX86(int operand,int machineRegOperand,int destReg);
    void LoadConstantAtomX86(RegisterDescription *registerDescription,int reg);
    void PushConstantAtomX86(RegisterDescription *registerDescription);
    void EmitUnconditionalBranchX86(BranchInstruction *branchInstruction);
    void EmitConditionalBranchX86(BranchInstruction *branchInstruction,bool twoOperands);
    void EmitPropertyBranchX86(BranchImmInstruction *branchInstruction);
    void EmitRegisterOperation3X86(Instruction *instruction);
    void EmitArgInX86(ArgInstruction *argInstruction);
    void EmitCompareOpFastPathX86(BranchInstruction *branchInstruction,unsigned char **branchToHelper1Ref,
                                  unsigned char **branchPastHelperRef,int icondOp);
    void EmitShrAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                unsigned char **branchPastHelperRef);
    void EmitAndAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelperRef,
                                unsigned char **branchPastHelperRef);
    void EmitAddAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                unsigned char **branchToHelper2Ref,
                                unsigned char **branchPastHelperRef);
    void EmitShlAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                unsigned char **branchToHelper2Ref,
                                unsigned char **branchPastHelperRef);
    void EmitRegisterOperation2X86(Instruction *instruction);
    void EmitArgOutX86(ArgInstruction *argInstruction);
    void FinishStdCall(int destReg,void *fn);
    void EmitLoadPropertyX86(IntConstInstruction *instruction);
    void EmitStorePropertyX86(IntConstInstruction *instruction);
    void EmitStoreBaseX86(int machineReg,int base,int offset);
    void EmitLoadBaseX86(int machineReg,int base,int offset);
    void EmitLoadElementI(Instruction *instruction);
    void EmitStoreElementI(StElemInstruction *instruction);
    void EmitSpillX86(Instruction *instruction);
    void EmitExchangeLocationsX86(Instruction *instruction);
    void EmitReloadX86(Instruction *instruction);
    void EmitNotAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                unsigned char **branchPastHelperRef);
    void EmitPrologueX86();
    void EmitEpilogueX86(int slotOffset);
    int  AssignFrameSlotX86(RegisterDescription *registerDescription);
    void SetMovBaseOps(int rd,int src,int imm,int scale);
    void AddOffsetX86(int reg,int offset);

    int AssignDestinationRegister(List<Instruction*> *entry,Instruction *defInstruction,Js::RegSlot byteCodeReg,MachineRegisters *machineRegisters,
        Instruction **prefix) {
        RegisterDescription *registerDescription=GetRegisterDescription(byteCodeReg);
        return registerDescription->AssignDestinationRegister(entry,defInstruction,machineRegisters,registerDescriptions,prefix);
    }

    int AssignSourceRegister(List<Instruction*> *useInstructionEntry,Instruction *useInstruction,Js::RegSlot byteCodeReg,
        MachineRegisters *machineRegisters,
        Instruction **prefix) {
        RegisterDescription *registerDescription=GetRegisterDescription(byteCodeReg);
        return registerDescription->AssignSourceRegister(useInstructionEntry,useInstruction,machineRegisters,registerDescriptions,prefix);
    }

    void FreeIfNotUsed(MachineRegisters *machineRegisters,Js::RegSlot sourceRegister,Instruction *curInstruction) {
        if (sourceRegister!=ReturnRegister) {
            RegisterDescription *registerDescription=GetRegisterDescription(sourceRegister);
            registerDescription->FreeIfNotUsed(machineRegisters,curInstruction);
        }
    }

    bool LegalRegisterAssignments(Instruction *instruction);

    void AssignMachineRegisters(List<Instruction*> *entry,MachineRegisters *machineRegisters,Instruction **prefix);
    void AssignMachineRegisters(BasicBlock *block,MachineRegisters *machineRegisters,int nRegs);
    void AssignMachineRegistersX86(int nRegs);
    void CharacterizeRegisters(int nRegs);
    void DescribeUses(Instruction *instruction);

    void AddInstructionToCurrentBlock(Instruction *instruction) {
        currentBasicBlock->AddInstruction(instruction);
        instructionCount++;
        DescribeUses(instruction);
    }

    Instruction *GetInstructionFromOffset(unsigned int offset) {
        Instruction *instruction=offsetToInstruction[offset-functionStartOffset];
        return instruction;
    }

    void SetInstructionAtOffset(unsigned int offset,Instruction *instruction) {
        offsetToInstruction[offset-functionStartOffset]=instruction;
    }

    void AddBranchReloc(unsigned int offset,BranchInstruction *branchInstruction) {
        Reloc *reloc=MakeStruct(alloc,Reloc);
        reloc->relocType=RelocTypeBranch;
        reloc->consumer=branchInstruction;
        reloc->offset=offset;
        ListFn<Reloc*>::Add(relocs,reloc,alloc);
    }

    // Dag section
    void LoadOperandFromStack(int spOffset,int destReg);
    void DagEmitCompareOpFastPathX86(BranchInstruction *branchInstruction,unsigned char **branchToHelper1Ref,
                                     unsigned char **branchPastHelperRef,int icondOp);
    void DagEmitAndAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelperRef,
                                   unsigned char **branchPastHelperRef);
    void DagEmitAddAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                   unsigned char **branchToHelper2Ref,
                                   unsigned char **branchPastHelperRef);
    void DagEmitConditionalBranchX86(BranchInstruction *branchInstruction,bool twoOperands);
    void EmitInstructionX86(Instruction *instruction);
    void DagEmitReloadX86(Dag *dag,int machineReg,int fpOffset);
    void DagEmitSpillX86(Dag *dag,int machineReg,int fpOffset);
    void DagEmitRegisterOperation3X86(Instruction *instruction);
    void DagEmitCallIX86(CallInstruction *callInstruction);
    void DagEmitNewScObjectX86(CallInstruction *callInstruction);
    void EmitPushEAX();
    void EmitMovReg(int dest,int src);
    void DagEmitRegisterOperation2X86(Instruction *instruction);
    void DagEmitLoadOperandX86(int srcReg);
    void DagEmitArgOutX86(ArgInstruction *argInstruction);
    void DagEmitArgInX86(ArgInstruction *argInstruction);
    void DagEmitPrefixOperands(Instruction *instruction);
    void DagEmitLoadPropertyX86(IntConstInstruction *instruction);
    void DagEmitStorePropertyX86(IntConstInstruction *instruction);
    void DagEmitLoadElementI(IntConstInstruction *instruction);
    void DagEmitStoreElementI(StElemInstruction *instruction);
};

extern Js::Atom OP_CmEq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);
extern Js::Atom OP_CmNeq_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);
extern Js::Atom OP_CmLt_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);
extern Js::Atom OP_CmLe_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);
extern Js::Atom OP_CmGt_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);
extern Js::Atom OP_CmGe_A(Js::Atom a,Js::Atom b,MemoryContext* memContext);

struct LoadPatch {
    void *type;
    int slotIndex;
};

#define LoadPatchInitialType 0xfafabead