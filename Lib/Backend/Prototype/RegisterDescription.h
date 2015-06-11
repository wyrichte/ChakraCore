/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class DagNode;

enum RegisterType {
    RegisterTypeUnknown,           // initial state
    RegisterTypeIntConstant,       // holds an integer constant
    RegisterTypeDoubleConstant,    // holds a double constant
    RegisterTypeObjectConstant,    // holds a constant object (such as undefined or global)
    RegisterTypeBlockScope,        // variable or temporary used only within a single block
    RegisterTypeFunctionScope      // variable or argument used across blocks
};

class RegisterDescription {
    RegisterType regType;
    Js::RegSlot index;                 // index of this register
    List<Instruction*> *accesses;      // list of accesses in function order
    ArenaAllocator *alloc;
    BasicBlock *localBlock;            // non-null if register defs and uses local to a single block
    double doubleConst;
    int intConst;
    Js::OpCode constDefOp;
    int argIndex;
    bool liveInFrameSlot;          // whether at the current code gen instruction, this register is in its frame slot
    int framePointerOffset;        // location on the stack, if needed (fixed once assigned)
    int location;                  // register location or NO_ASSIGNMENT (changes during code generation)
    DagNode *dagNode;
    int nAccesses;
    List<Instruction*> *lastAccessEntry;  // list entry in accesses for last access (during reg. assignment)
public:
    static const int NoFramePointerOffset=(-1);
    static const int BigOrdinal=1000000;
    RegisterDescription(ArenaAllocator *alloc,Js::RegSlot index) : regType(RegisterTypeUnknown),index(index), accesses(0),
        alloc(alloc), localBlock(0), doubleConst(0), intConst(0), constDefOp(0), argIndex(NO_ASSIGNMENT), 
        liveInFrameSlot(false), framePointerOffset(NoFramePointerOffset), location(NO_ASSIGNMENT), 
        dagNode(null),  nAccesses(-1), lastAccessEntry(0) {
        accesses=ListFn<Instruction*>::MakeListHead(alloc);
    }

    void SetFramePointerOffset(int offset) {
        framePointerOffset=offset;
    }

    int GetFramePointerOffset() {
        return framePointerOffset;
    }

    void SetDagNode(DagNode *dagNode) {
        this->dagNode=dagNode;
    }

    int NextAccessOrdinal() {
        BasicBlock *block=lastAccessEntry->data->basicBlock;
        if ((lastAccessEntry==NULL)||(lastAccessEntry->next->isHead)) {
            return BigOrdinal;
        }
        else if (lastAccessEntry->next->data->basicBlock==block)
            return lastAccessEntry->next->data->ordinal;
        else return BigOrdinal;
    }

    int NRemainingAccesses();
    void ClearDagNode() {
        dagNode=null;
    }

    DagNode *GetDagNode() {
        return dagNode;
    }
    void SetTemp() {
        regType=RegisterTypeBlockScope;
    }

    RegisterType GetRegisterType() {
        return regType;
    }

    int GetIntConst() {
        return intConst;
    }

    int GetNumberOfAccesses(BasicBlock *block);

    double GetDoubleConst() {
        return doubleConst;
    }

    Js::OpCode GetConstDefOp() {
        return constDefOp;
    }

    void AddUse(Instruction *instruction) {
        ListFn<Instruction*>::Add(accesses,instruction,alloc);
    }

    void AddDef(Instruction *instruction) {
        ListFn<Instruction*>::Add(accesses,instruction,alloc);
    }

    bool IsDef(Instruction *instruction) {
        return instruction->rd==index;
    }

    bool IsArgIn(Instruction *instruction) {
       return  (instruction->op==Js::OpCode::ArgIn_A)||(instruction->op==Js::OpCode::ArgInI_A);
    }

    bool HasDefInBlock(BasicBlock *block);
    bool FirstAccessIsUse(BasicBlock *block);
    void NewBlock() {
        ClearLocation();
        ClearDagNode();
        nAccesses=(-1);
    }

    void ClearLocation() {
        location=NO_ASSIGNMENT;
    }

    int GetLocation() {
        return location;
    }

    void SetLocation(int loc) {
        location=loc;
    }

    Instruction *GetNextUse(Instruction *instruction);
    MachineRegister *CreateFreeReg(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
                                   Instruction *useInstruction);
    MachineRegister *GetVarRegister(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
                                    Instruction *useInstruction);
    MachineRegister *Reload(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
                            Instruction *useInstruction);
    MachineRegister *GetTempRegister(Instruction *defInstruction,MachineRegisters *machineRegisters,
                                     RegisterDescription **registerFile,Instruction **prefix,Instruction *useInstruction);
    void FreeIfNotUsed(MachineRegisters *machineRegisters,Instruction *curInstruction);
    int AssignSourceRegister(List<Instruction*> *useInstructionEntry,Instruction *useInstruction,MachineRegisters *machineRegisters,RegisterDescription **registerFile,
                             Instruction **prefix);
    inline static bool IsConstRegType(RegisterType regType) {
        return ((regType==RegisterTypeIntConstant)||(regType==RegisterTypeDoubleConstant)||(regType==RegisterTypeObjectConstant));
    }

    bool IsTemporaryDef(List<Instruction*> *entry,Instruction *defInstruction,RegisterDescription **registerFile);
    bool IsArgOutDef(Instruction *defInstruction);
    int AssignDestinationRegister(List<Instruction*> *entry,Instruction *defInstruction,MachineRegisters *machineRegisters,RegisterDescription **registerFile,
                                  Instruction **prefix);
    void SetArgIn(int argIndex,Instruction *defInstruction);
    void Characterize();
    void SetConstant(double d,Instruction *defInstruction) {
        regType=RegisterTypeDoubleConstant;
        doubleConst=d;
        AddDef(defInstruction);
        constDefOp=defInstruction->op;
    }

    void SetConstant(Instruction *defInstruction) {
        regType=RegisterTypeObjectConstant;
        AddDef(defInstruction);
        constDefOp=defInstruction->op;
    }

    void SetConstant(int i,Instruction *defInstruction) {
        regType=RegisterTypeIntConstant;
        intConst=i;
        AddDef(defInstruction);
        constDefOp=defInstruction->op;
    }
};
