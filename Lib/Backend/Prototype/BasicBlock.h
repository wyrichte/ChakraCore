/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class Dag;
class MachineRegisters;

class BasicBlock {
    List<BasicBlock*> *predecessors;
    List<BasicBlock*> *successors;
    List<Instruction*> *instructions;
    ArenaAllocator *alloc;
    int startOffset;
    unsigned int nativeStartOffset;
    Dag *dag;
public:
    BasicBlock(ArenaAllocator *alloc,int startOffset) : alloc(alloc), startOffset(startOffset), nativeStartOffset(0), dag(NULL) {
        instructions=ListFn<Instruction*>::MakeListHead(alloc);
        predecessors=ListFn<BasicBlock*>::MakeListHead(alloc);
        successors=ListFn<BasicBlock*>::MakeListHead(alloc);
    }

    Dag* GetDag() {
        return dag;
    }

    void SetDag(Dag *dag) {
        this->dag=dag;
    }

    void AddInstruction(Instruction *instruction) {
        ListFn<Instruction*>::Add(instructions,instruction,alloc);
        instruction->basicBlock=this;
    }

    void AddSuccessor(BasicBlock *successor) {
        ListFn<BasicBlock*>::Add(successors,successor,alloc);
        successor->AddPredecessorOneWay(this);
    }

    void AddOpCodeCounts(OpCount *opHistogram,int nRuns) {
        for (List<Instruction*> *entry=instructions->next;!(entry->isHead);entry=entry->next) {
            opHistogram[entry->data->op].count+=nRuns;
        }
    }

    int GetStartOffset() {
        return startOffset;
    }

    unsigned int GetNativeStartOffset() {
        return nativeStartOffset;
    }

    void SetNativeStartOffset(unsigned int offset) {
        nativeStartOffset=offset;
    }

    void AllocateRegisters(MachineRegisters *machineRegisters,RegisterDescription **registerDescriptions,int nLocations);

    int NumberInstructions();

    static void PrintInstruction(Instruction *instruction,MemoryContext* memContext);
    static void PrintInstructionSimple(Instruction *instruction,int indentAmt,MemoryContext* memContext);

    List<Instruction*> *GetInstructions() {
        return instructions;
    }
    void Print(MemoryContext* memContext);
    BasicBlock *SplitIfNecessary(Instruction *instruction,int targetOffset);
    void AddPredecessorOneWay(BasicBlock *predecessor) {
        ListFn<BasicBlock*>::Add(predecessors,predecessor,alloc);
    }
};

