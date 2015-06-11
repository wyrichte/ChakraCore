/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
extern void Indent(int indentAmt);
class RTLGenerator;

enum DagNodeType {
    DagNodeIntConst,
    DagNodeDoubleConst,
    DagNodeLocation,
    DagNodeInstruction
};

class DagNode {
protected:
    DagNode **operands;
    List<DagNode*> *constraints;
    List<DagNode*> *uses;
    List<DagNode*> *indirectUses;
    BitArray *attachedLocations;
    ArenaAllocator *alloc;
    int nOperands;
    DagNodeType nodeType;
    bool emitted;
    bool live;
    bool isListed;
    int label;
    int markKey;
public:
    DagNode(int nLocations, ArenaAllocator *alloc,int label) : alloc(alloc), emitted(false), live(false),
        label(label), operands(NULL), isListed(false),markKey(0) {
        attachedLocations=Anew(alloc,BitArray,nLocations,alloc);
        constraints=ListFn<DagNode*>::MakeListHead(alloc);
        uses=ListFn<DagNode*>::MakeListHead(alloc);
        indirectUses=ListFn<DagNode*>::MakeListHead(alloc);
        nodeType=DagNodeLocation;
    }

    void SetEmitted(bool b) {
        emitted=b;
    }

    bool GetEmitted() {
        return emitted;
    }

    bool IsRoot() {
        return ListFn<DagNode*>::Empty(uses)&&ListFn<DagNode*>::Empty(indirectUses);
    }

    void AddUsesToQ(List<DagNode*> *visitQ,int curMarkKey) {
        for (List<DagNode*> *entry=uses->next;!(entry->isHead);entry=entry->next) {
            DagNode *node=entry->data;
            if (!(node->IsMarked(curMarkKey)))
                ListFn<DagNode*>::Add(visitQ,node,alloc);
        }
        for (List<DagNode*> *entry=indirectUses->next;!(entry->isHead);entry=entry->next) {
            DagNode *node=entry->data;
            if (!(node->IsMarked(curMarkKey)))
                ListFn<DagNode*>::Add(visitQ,node,alloc);
        }
    }

    bool AllParentsListed() {
        for (List<DagNode*> *entry=uses->next;!(entry->isHead);entry=entry->next) {
            DagNode *node=entry->data;
            if (!node->isListed)
                return false;
        }
        for (List<DagNode*> *entry=indirectUses->next;!(entry->isHead);entry=entry->next) {
            DagNode *node=entry->data;
            if (!node->isListed)
                return false;
        }
        return true;
    }

    bool HasMultipleUses() {
        return ListFn<DagNode*>::Count(uses)>1;
    }

    bool IsMarked(int currentMark) {
        return markKey==currentMark;
    }

    void Mark(int currentMark) {
        markKey=currentMark;
    }

    bool IsListed() {
        return isListed;
    }

    void SetIsListed(List<DagNode*> *readyList) {
        isListed=true;
        for (int i=0;i<nOperands;i++) {
            DagNode *node=operands[i];
            if ((!(node->isListed||node->IsLeaf()))&&node->AllParentsListed())
                ListFn<DagNode*>::Add(readyList,node,alloc);
        }
        for (List<DagNode*> *entry=constraints->next;!(entry->isHead);entry=entry->next) {
            DagNode *node=entry->data;
            if ((!(node->isListed||node->IsLeaf()))&&node->AllParentsListed())
                ListFn<DagNode*>::Add(readyList,node,alloc);
        }
    }

    bool IsLeaf() {
        return (operands==NULL)&&(ListFn<DagNode*>::Empty(constraints));
    }

    DagNodeType GetNodeType() {
        return nodeType;
    }

    DagNode **GetOperands() {
        return operands;
    }

    virtual void Print(int indentAmt) {
        Indent(indentAmt);
        printf("Location %d\n",label);
    }

    int GetLabel() {
        return label;
    }

    void RemoveLocation(unsigned int loc) {
        attachedLocations->ClearBit(loc);
    }

    void AddLocation(unsigned int loc) {
        attachedLocations->SetBit(loc);
    }

    void AddUse(DagNode *afterThis) {
        ListFn<DagNode*>::Add(uses,afterThis,alloc);
    }

    void AddIndirectUse(DagNode *afterThis) {
        ListFn<DagNode*>::Add(indirectUses,afterThis,alloc);
    }

    void AddConstraint(DagNode *beforeThis) {
        ListFn<DagNode*>::Add(constraints,beforeThis,alloc);
        beforeThis->AddIndirectUse(this);
    }

    void AddOperands1(DagNode *a) {
        operands=(DagNode**)alloc->Alloc(sizeof(DagNode*));
        operands[0]=a;
        nOperands=1;
        a->AddUse(this);
    }

    void AddOperands2(DagNode *a,DagNode *b) {
        operands=(DagNode**)alloc->Alloc(sizeof(DagNode*)*2);
        operands[0]=a;
        operands[1]=b;
        a->AddUse(this);
        b->AddUse(this);
        nOperands=2;
    }

    void AddOperands3(DagNode *a,DagNode *b,DagNode *c) {
        operands=(DagNode**)alloc->Alloc(sizeof(DagNode*)*3);
        operands[0]=a;
        operands[1]=b;
        operands[2]=c;
        a->AddUse(this);
        b->AddUse(this);
        c->AddUse(this);
        nOperands=3;
    }
};

class IntConstDagNode : public DagNode {
    int value;
    bool toAtom;
public:
    IntConstDagNode(int nLocations, ArenaAllocator *alloc,int label,int value,bool toAtom) : DagNode(nLocations,alloc,label), value(value), toAtom(toAtom) {
        nodeType=DagNodeIntConst;
    }

    void Print(int indentAmt) {
        Indent(indentAmt);
        printf("Int: %d\n",value);
    }

    int GetValue() {
        return value;
    }
};

class DoubleConstDagNode : public DagNode {
    double value;
public:
    DoubleConstDagNode(int nLocations,ArenaAllocator *alloc,int label,double value) : DagNode(nLocations,alloc,label), value(value) {
        nodeType=DagNodeIntConst;
    }

    void Print(int indentAmt) {
        Indent(indentAmt);
        printf("Double: %f\n",value);
    }

    double GetValue() {
        return value;
    }
};

class InstructionDagNode : public DagNode {
    Instruction *instruction;
public: 
    InstructionDagNode(int nLocations,ArenaAllocator *alloc,Instruction *instruction) : DagNode(nLocations,alloc,instruction->rd), 
        instruction(instruction) {
        nodeType=DagNodeInstruction;
    }

    void Print(int indentAmt,MemoryContext* memContext) {
        BasicBlock::PrintInstructionSimple(instruction,indentAmt,memContext);
    }

    Instruction *GetInstruction() {
        return instruction;
    }

    void EmitRequirements(Dag *dag,int indentAmt,RTLGenerator *generator);
    void PrintEmitRequirements(Dag *dag,int indentAmt,RTLGenerator *generator);

    static bool PushInstructionOperands(Instruction *instruction);

    void EnsureSingleOperandInEAX(Dag *dag,RTLGenerator *generator);

    bool IsBranch() {
        return instruction->isBranchInstruction;
    }
};

struct CallInfo {
    DagNode *callStartNode;
    List<DagNode*> *argOutNodes;
};

class Dag {
    List<DagNode*> *nodes;
    List<CallInfo*> *startCalls;  // stack of start call nodes; each arg out depends on top start call node
    BitArray **currentAssignments;  // map machine register to enregistered pseudo-regs
    ArenaAllocator *alloc;
    MemoryContext* memContext;
    int nLocations;
    RegisterDescription **registerDescriptions;
    int markKey;
    InstructionDagNode *blockEndNode;
    DagNode **lastPropertyUpdate;
    DagNode *lastOblitStoreElemI;
    long nPropertys;
    Instruction **emitOrder;
    int nEmitInstructions;
    int nActualInstructions;
public:
    bool silent;
    Dag(ArenaAllocator *alloc,MemoryContext* memContext,int nLocations,RegisterDescription **registerDescriptions,bool silent) : alloc(alloc),
        memContext(memContext), nLocations(nLocations),
        registerDescriptions(registerDescriptions), markKey(1), blockEndNode(null), silent(silent), lastOblitStoreElemI(NULL) {
        nodes=ListFn<DagNode*>::MakeListHead(alloc);
        startCalls=ListFn<CallInfo*>::MakeListHead(alloc);
        currentAssignments=(BitArray**)alloc->Alloc(sizeof(BitArray*)*nLocations);
        for (int i=0;i<nLocations;i++) {
            currentAssignments[i]=Anew(alloc,BitArray,nLocations,alloc);
        }
        nPropertys=Js::ScriptContext::Info(memContext)->GetLastPropertyId()+1;
        lastPropertyUpdate=(DagNode **)alloc->Alloc(nPropertys*sizeof(DagNode*));
        for (int j=0;j<nPropertys;j++)
            lastPropertyUpdate[j]=NULL;
    }

    void AddBlockEnd(Instruction *instruction);
    void AddNewScObjectSimple(CallInstruction *callInstruction);
    void AddLoadElementI(IntConstInstruction *loadInstruction);
    void AddStoreElementI(StElemInstruction *stElemInstruction);
    void AddStoreProperty(IntConstInstruction *loadInstruction);
    void AddLoadProperty(IntConstInstruction *loadInstruction);
    void AddInitializeInstructionLeaf(Instruction *instruction);
    void AddArgIn(ArgInstruction *argInstruction);
    void AddArgOut(ArgInstruction *argInstruction);
    void AddInstructionLeaf(Instruction *instruction);
    void AddInstruction(Instruction *instruction);
    void AddRegisterOperation3(Instruction *instruction);
    void AddRegisterOperation2(Instruction *instruction);
    InstructionDagNode *AddSingleOperand(Instruction *instruction);
    InstructionDagNode *AddOperandPair(Instruction *instruction);
    InstructionDagNode *AddOperandTriple(StElemInstruction *stElemInstruction);
    void AddConditionalBranch1(BranchInstruction *branchInstruction);
    void AddConditionalBranch2(BranchInstruction *branchInstruction);
    void AddCall(CallInstruction *callInstruction);
    void AddMove(Instruction *movInstruction);
    void UpdateDestination(DagNode *node,Instruction *instruction);
    void PushStartCallLeaf(Instruction *instruction);

    void PrintEmitInstruction(InstructionDagNode *idagNode,int indentAmt,RTLGenerator *generator);
    void PrintEmitNode(DagNode *node,int indentAmt,RTLGenerator *generator);
    void PrintEmitOrder(BasicBlock *block,RTLGenerator *generator);

    void PrepareRegisterOperation3(Instruction *instruction);

    void AddNode(DagNode *node) {
        ListFn<DagNode*>::Add(nodes,node,alloc);
    }

    InstructionDagNode *AddInstructionNode(Instruction *instruction) {
        InstructionDagNode *opNode=Anew(alloc,InstructionDagNode,nLocations,alloc,instruction);
        AddNode(opNode);
        return opNode;
    }

    DagNode *Node(int reg) {
        AssertMsg(reg<nLocations,"Register out of range");
        return registerDescriptions[reg]->GetDagNode();
    }

    void SetNode(int destReg,DagNode *node) {
        registerDescriptions[destReg]->SetDagNode(node);
    }

    DagNode* EnsureNode(int srcReg,Instruction *instruction);

    static Dag *BuildDag(BasicBlock *block,int nLocations,RegisterDescription **registerDescriptions,ArenaAllocator *alloc,
        MemoryContext* memContext,bool silent) {
        Dag *dag=Anew(alloc,Dag,alloc,memContext,nLocations,registerDescriptions,silent);
        List<Instruction*> *instructions=block->GetInstructions();
        for (List<Instruction*> *entry=instructions->next;!(entry->isHead);entry=entry->next) {
            dag->AddInstruction(entry->data);
        }
        return dag;
    }

    void EmitNode(DagNode *node,int indentAmt,RTLGenerator *generator,bool pushNode);
    void EmitBlock(BasicBlock *block,RTLGenerator *generator);
    void EmitCoreInstruction(Instruction *instruction,RTLGenerator *generator);
    void EmitInstruction(InstructionDagNode *idagNode,int indentAmt,RTLGenerator *generator);
    void LoadNodeIntoReg(DagNode *node,RTLGenerator *generator,int reg);
};
