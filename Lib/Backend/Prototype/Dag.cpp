/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

DagNode* Dag::EnsureNode(int srcReg,Instruction *instruction) {
    DagNode *node=Node(srcReg);
    if (node==null) {
        RegisterDescription *registerDescription=registerDescriptions[srcReg];
        switch (registerDescription->GetRegisterType()) {
            case RegisterTypeBlockScope:
            case RegisterTypeFunctionScope:
                node=Anew(alloc,DagNode,nLocations,alloc,srcReg);
                break;
            case RegisterTypeObjectConstant: {
                AssertMsg(registerDescription->GetConstDefOp()==Js::OpCode::LdC_A_Null,"this should only be load null");
                node=Anew(alloc,IntConstDagNode,nLocations,alloc,srcReg,0,false);
                break;
            }
            case RegisterTypeIntConstant: {
                AssertMsg(registerDescription->GetConstDefOp()!=Js::OpCode::LdC_A_Null,"this should be an obj const");
                node=Anew(alloc,IntConstDagNode,nLocations,alloc,srcReg,
                    registerDescription->GetIntConst(),true);
                break;
            }
            case RegisterTypeDoubleConstant: {
                node=Anew(alloc,DoubleConstDagNode,nLocations,alloc,srcReg,
                    registerDescription->GetDoubleConst());
                break;
            }
            default:
                AssertMsg(false,"Unimplemented register type");
                break;
        }
        AddNode(node);
        registerDescriptions[srcReg]->SetDagNode(node);
    }
    return node;
}

void Dag::AddBlockEnd(Instruction *instruction) {
    InstructionDagNode *dagNode=Anew(alloc,InstructionDagNode,nLocations,alloc,instruction);
    blockEndNode=dagNode;
}

void Dag::PushStartCallLeaf(Instruction *instruction) {
    InstructionDagNode *opNode=AddInstructionNode(instruction);
    CallInfo *callInfo=MakeStruct(alloc,CallInfo);
    callInfo->argOutNodes=ListFn<DagNode*>::MakeListHead(alloc);
    callInfo->callStartNode=opNode;
    ListFn<CallInfo*>::Push(startCalls,callInfo,alloc);
}

void Dag::AddInitializeInstructionLeaf(Instruction *instruction) {
    InstructionDagNode *opNode=AddInstructionNode(instruction);
    UpdateDestination(opNode,instruction);
}

InstructionDagNode *Dag::AddOperandPair(Instruction *instruction) {
    DagNode* rs1Node=EnsureNode(instruction->rs1,instruction);
    DagNode* rs2Node=EnsureNode(instruction->rs2,instruction);
    InstructionDagNode *opNode=AddInstructionNode(instruction);
    opNode->AddOperands2(rs1Node,rs2Node);
    return opNode;
}

InstructionDagNode *Dag::AddOperandTriple(StElemInstruction *stElemInstruction) {
    DagNode* rs1Node=EnsureNode(stElemInstruction->rs1,stElemInstruction);
    DagNode* rs2Node=EnsureNode(stElemInstruction->rs2,stElemInstruction);
    DagNode* valNode=EnsureNode(stElemInstruction->val,stElemInstruction);
    InstructionDagNode *opNode=AddInstructionNode(stElemInstruction);
    opNode->AddOperands3(rs1Node,rs2Node,valNode);
    return opNode;
}

InstructionDagNode *Dag::AddSingleOperand(Instruction *instruction) {
    DagNode* rs1Node=EnsureNode(instruction->rs1,instruction);
    InstructionDagNode *opNode=AddInstructionNode(instruction);
    opNode->AddOperands1(rs1Node);
    return opNode;
}

void Dag::AddConditionalBranch2(BranchInstruction *branchInstruction) {
    AddOperandPair(branchInstruction);
}

void Dag::AddConditionalBranch1(BranchInstruction *branchInstruction) {
    AddSingleOperand(branchInstruction);
}

void Dag::UpdateDestination(DagNode *node,Instruction *instruction) {
    int destReg=instruction->rd;
    DagNode *oldNode=Node(destReg);
    if (oldNode!=null)
        oldNode->RemoveLocation(destReg);
    node->AddLocation(destReg);
    SetNode(destReg,node);
}

void Dag::AddRegisterOperation3(Instruction *instruction) {
    InstructionDagNode *opNode=AddOperandPair(instruction);
    UpdateDestination(opNode,instruction);
}

void Dag::AddRegisterOperation2(Instruction *instruction) {
    InstructionDagNode *opNode=AddSingleOperand(instruction);
    UpdateDestination(opNode,instruction);
}

void Dag::AddCall(CallInstruction *callInstruction) {
    InstructionDagNode *opNode=AddSingleOperand(callInstruction);
    UpdateDestination(opNode,callInstruction);
    // TODO for cse: kill any ldfld and ldelem nodes
    AssertMsg(!ListFn<CallInfo*>::Empty(startCalls),"When adding a call, there should be a start call on the stack");
    CallInfo *callInfo=ListFn<CallInfo*>::PopEntry(startCalls)->data;
    if (!(ListFn<DagNode*>::Empty(callInfo->argOutNodes))) {
        DagNode *prev=callInfo->argOutNodes->prev->data;
        opNode->AddConstraint(prev);
    }
    else {
        AssertMsg(false,"there should always be at least one argument (this)");
        opNode->AddConstraint(callInfo->callStartNode);
    }
    // TODO: add call-to-call chaining
}

void Dag::AddNewScObjectSimple(CallInstruction *callInstruction) {
    InstructionDagNode *opNode=AddSingleOperand(callInstruction);
    UpdateDestination(opNode,callInstruction);
}

void Dag::AddMove(Instruction *movInstruction) {
    DagNode *srcNode=EnsureNode(movInstruction->rs1,movInstruction);
    UpdateDestination(srcNode,movInstruction);
}

void Dag::AddArgIn(ArgInstruction *argInstruction) {
    // TODO: make sure correct frame offset is used
    AddInitializeInstructionLeaf(argInstruction);
}

void Dag::AddArgOut(ArgInstruction *argInstruction) {
    // treat as single operand use
    InstructionDagNode *opNode=AddSingleOperand(argInstruction);
    CallInfo *callInfo=ListFn<CallInfo*>::Top(startCalls);
    if (!(ListFn<DagNode*>::Empty(callInfo->argOutNodes))) {
        DagNode *prev=callInfo->argOutNodes->prev->data;
        opNode->AddConstraint(prev);
    }
    else opNode->AddConstraint(callInfo->callStartNode);
    ListFn<DagNode*>::Add(callInfo->argOutNodes,opNode,alloc);
}

void Dag::AddLoadProperty(IntConstInstruction *loadInstruction) {
    // TODO: remember and constrain for cse and ordering
    // TODO: remember specific field offset (or direct array offset)
    DagNode *opNode=AddSingleOperand(loadInstruction);
    UpdateDestination(opNode,loadInstruction);
    if (loadInstruction->op==Js::OpCode::LdFld) {
        DagNode *storeNode=lastPropertyUpdate[loadInstruction->immInt];
        if (storeNode!=NULL)
            opNode->AddConstraint(storeNode);
        if (lastOblitStoreElemI!=NULL) {
            opNode->AddConstraint(lastOblitStoreElemI);
        }
    }
}

void Dag::AddStoreProperty(IntConstInstruction *storeInstruction) {
    // TODO: remember and constrain for cse and ordering
    // TODO: remember specific field offset (or direct array offset)
    DagNode *node=AddOperandPair(storeInstruction);
    if (lastPropertyUpdate[storeInstruction->immInt]!=NULL) {
        node->AddConstraint(lastPropertyUpdate[storeInstruction->immInt]);
    }
    lastPropertyUpdate[storeInstruction->immInt]=node;
    // stores must be in order
    if (lastOblitStoreElemI!=NULL) {
        node->AddConstraint(lastOblitStoreElemI);
    }
}

void Dag::AddLoadElementI(IntConstInstruction *loadInstruction) {
    // TODO: remember and constrain for cse and ordering
    // TODO: remember specific field offset (or direct array offset)
    DagNode *opNode=AddOperandPair(loadInstruction);
    UpdateDestination(opNode,loadInstruction);
    if (lastOblitStoreElemI!=NULL) {
        opNode->AddConstraint(lastOblitStoreElemI);
    }
    // TODO: something faster
    for (int i=0;i<nPropertys;i++) {
        if (lastPropertyUpdate[i]!=NULL) {
            opNode->AddConstraint(lastPropertyUpdate[i]);
        }
    }
}

void Dag::AddStoreElementI(StElemInstruction *stElemInstruction) {
    DagNode *node=AddOperandTriple(stElemInstruction);
    if (lastOblitStoreElemI!=NULL) {
        node->AddConstraint(lastOblitStoreElemI);
    }
    lastOblitStoreElemI=node;
}

void Dag::AddInstruction(Instruction *instruction) {
    Js::OpCode op=instruction->op;
    switch (op) {
       case Js::OpCode::FunctionExit: {
           AddBlockEnd(instruction);
           break;
       }
       case Js::OpCode::Nop: {
           break;
       }
       case Js::OpCode::Ret: {
           AddBlockEnd(instruction);
           break;
       }
       case Js::OpCode::BrEq_A: 
       case Js::OpCode::BrNeq_A: 
       case Js::OpCode::BrGt_A: 
       case Js::OpCode::BrLt_A: 
       case Js::OpCode::BrGe_A: 
       case Js::OpCode::BrLe_A: 
           AddConditionalBranch2((BranchInstruction*)instruction);
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
           AddRegisterOperation3(instruction);
           break;
       case Js::OpCode::NewScObject:
       case Js::OpCode::CallI:
           AddCall((CallInstruction*)instruction);
           break;
       case Js::OpCode::NewScObjectSimple:
           AddNewScObjectSimple((CallInstruction*)instruction);
           break;
       case Js::OpCode::Ld_A:
           // for now, just treat move like any other operation
           //AddMove(instruction);
           AddRegisterOperation2(instruction);
           break;
       case Js::OpCode::Neg_A:
       case Js::OpCode::Not_A:
       case Js::OpCode::LdLen_A:
       case Js::OpCode::NewArray:
       case Js::OpCode::NewScArray:
       case Js::OpCode::GetEnumerator:
       case Js::OpCode::GetCurrent:
           AddRegisterOperation2(instruction);
           break;
       case Js::OpCode::Br:
           AddBlockEnd(instruction);
           break;
       case Js::OpCode::LdRoot:
       case Js::OpCode::LdUndef:
       case Js::OpCode::LdTrue:
       case Js::OpCode::LdFalse:
           AddInitializeInstructionLeaf(instruction);
           break;
       case Js::OpCode::LdC_A_Null:
       case Js::OpCode::LdC_A_I4_0:
       case Js::OpCode::LdC_A_I4_1:
       case Js::OpCode::LdC_A_I4:
       case Js::OpCode::LdC_A_R8:
       case Js::OpCode::LdC_A_R8_0:
           // these are handled when an instruction asks for
           // the value
           break;
       case Js::OpCode::BrFalse_A:
       case Js::OpCode::BrTrue_A:
       case Js::OpCode::BrOnEmpty:
           AddConditionalBranch1((BranchInstruction*)instruction);
           break;
       case Js::OpCode::ArgIn_A:
           AddArgIn((ArgInstruction*)instruction);
           break;
       case Js::OpCode::TempArgOut_A:
           AddArgOut((ArgInstruction*)instruction);
           break;
       case Js::OpCode::ArgOut_A:
           // Using TempArgOut instead
           break;
       case Js::OpCode::LdElemC:
       case Js::OpCode::LdFld:
       case Js::OpCode::NewScFunc:
           AddLoadProperty((IntConstInstruction*)instruction);
           break;
       case Js::OpCode::StFld:
       case Js::OpCode::StElemC:
           AddStoreProperty((IntConstInstruction*)instruction);
           break;
       case Js::OpCode::LdElemI_A:
           AddLoadElementI((IntConstInstruction*)instruction);
           break;
       case Js::OpCode::StElemI_A:
           AddStoreElementI((StElemInstruction*)instruction);
           break;
       case Js::OpCode::LdStr: {
           AddInitializeInstructionLeaf(instruction);
           break;
       }
       case Js::OpCode::NewRegEx: {
           AddInitializeInstructionLeaf(instruction);
           break;
       }
       case Js::OpCode::StartCall: {
           // TODO: keep track of arguments related to call if necessary
           PushStartCallLeaf(instruction);
           break;
       }
       case Js::OpCode::LdEnv:
           AddInitializeInstructionLeaf(instruction);
           break;
       default: {
           AssertMsg(false,"Unimplemented opcode");
           break;
       }
   }
}

/*
void Indent(int indentAmt) {
    for (int i=0;i<indentAmt;i++)
        putchar(' ');
}
*/

void InstructionDagNode::PrintEmitRequirements(Dag *dag,int indentAmt,RTLGenerator *generator) {
    bool reqPrint=false;
    if (!ListFn<DagNode*>::Empty(constraints)) {
        Indent(indentAmt);
        printf("Requirements for instruction %d\n",instruction->ordinal);
        reqPrint=true;
        Indent(indentAmt+2);
        printf("Constraints...\n");
        for (List<DagNode*> *entry=constraints->next;!(entry->isHead);entry=entry->next) {
            dag->PrintEmitNode(entry->data,indentAmt+4,generator);
        }
    }
    if (nOperands>0) {
        if (!reqPrint) {
            Indent(indentAmt);
            printf("Requirements for instruction %d\n",instruction->ordinal);
        }
        Indent(indentAmt+2);
        printf("Operands...\n");
        for (int i=nOperands-1;i>=0;i--) {
            dag->PrintEmitNode(operands[i],indentAmt+4,generator);
        }
    }
}

void Dag::PrintEmitInstruction(InstructionDagNode *idagNode,int indentAmt,RTLGenerator *generator) {
    Instruction *instruction=idagNode->GetInstruction();
    RegisterDescription *registerDescription=registerDescriptions[instruction->rd];
    int location=registerDescription->GetLocation();
    if (idagNode->GetEmitted()) {
        Indent(indentAmt);
        printf("ref instruction %d",instruction->ordinal);
        if (location!=NO_ASSIGNMENT) {
            printf(" from machine register %s\n",RegName(location));
        }
        else printf(" from frame slot %d(ebp)\n",registerDescription->GetFramePointerOffset());
    }
    else {
        idagNode->SetEmitted(true);
        idagNode->PrintEmitRequirements(this,indentAmt,generator);
        emitOrder[nActualInstructions++]=instruction;
        idagNode->Print(indentAmt,generator->GetMemoryContext());
        // TODO: only save to frame on last def if doesn't have multiple uses
        if (idagNode->HasMultipleUses()||
            ((location==NO_ASSIGNMENT)&&(registerDescription->GetRegisterType()==RegisterTypeFunctionScope))) {
            Indent(indentAmt+2);
            if (location!=NO_ASSIGNMENT) {
                printf("  save to machine register %s\n",RegName(location));
            }
            else printf("  save to frame slot %d(ebp)\n",generator->AssignFrameSlotX86(registerDescription));
        }
    }
}

void Dag::PrintEmitNode(DagNode *node,int indentAmt,RTLGenerator *generator) {
    switch (node->GetNodeType()) {
        case DagNodeIntConst:
        case DagNodeDoubleConst:
            node->Print(indentAmt);
            break;
        case DagNodeLocation: {
            int label=node->GetLabel();
            Indent(indentAmt);
            RegisterDescription *registerDescription=registerDescriptions[label];
            int location=registerDescription->GetLocation();
            if (location!=NO_ASSIGNMENT) {
                printf(" location %d from machine register %s\n",label,RegName(location));
            }
            else printf(" location %d from frame slot %d(ebp)\n",label,registerDescription->GetFramePointerOffset());
            break;
        }
        case DagNodeInstruction: {
            InstructionDagNode *opNode=(InstructionDagNode *)node;
            PrintEmitInstruction(opNode,indentAmt,generator);
            break;
        }
    }
}

void Dag::PrintEmitOrder(BasicBlock *block,RTLGenerator *generator) {
    for (int i=0;i<nLocations;i++) {
        registerDescriptions[i]->NewBlock();
    }
    nEmitInstructions=block->NumberInstructions();
    MachineRegisters *x86MachineRegisters=Anew(alloc,MachineRegisters,alloc,X86_NINTREGS,X86_TEMP_REGISTER,X86_EDX,X86_FRAME_POINTER,
        X86_STACK_POINTER,X86_CALLEE_SAVED);
    block->AllocateRegisters(x86MachineRegisters,registerDescriptions,nLocations);
    emitOrder=(Instruction **)alloc->Alloc(nEmitInstructions*sizeof(Instruction *));
    nActualInstructions=0;
    List<DagNode*> *q=ListFn<DagNode*>::MakeListHead(alloc);
    for (List<DagNode*> *entry=nodes->next;!(entry->isHead); entry=entry->next) {
        if (entry->data->IsRoot())
            ListFn<DagNode*>::Add(q,entry->data,alloc);
    }
    printf("Dag order for block at start offset 0x%x\n",block->GetStartOffset());
    // reload global registers
    for (int i=1;i<nLocations;i++) {
        RegisterDescription *registerDescription=registerDescriptions[i];
        if ((registerDescription->GetRegisterType()==RegisterTypeFunctionScope)&&
            (registerDescription->GetLocation()!=NO_ASSIGNMENT)) {
                printf("  reload %s = L%d at %d(ebp)\n",RegName(registerDescription->GetLocation()),i,
                    registerDescription->GetFramePointerOffset());
        }
    }
    while (!(ListFn<DagNode*>::Empty(q))) {
        DagNode *node=ListFn<DagNode*>::PopEntry(q)->data;
        PrintEmitNode(node,2,generator);
    }
    if (blockEndNode!=null) {
        PrintEmitNode(blockEndNode,2,generator);
    }
    for (int i=1;i<nLocations;i++) {
        RegisterDescription *registerDescription=registerDescriptions[i];
        if ((registerDescription->GetRegisterType()==RegisterTypeFunctionScope)&&
            (registerDescription->GetLocation()!=NO_ASSIGNMENT)&&
            (registerDescription->HasDefInBlock(block))) {
                printf("  L%d at %d(ebp) = %s\n",i,generator->AssignFrameSlotX86(registerDescription),
                    RegName(registerDescription->GetLocation()));
        }
    }
    printf("with final sequence...\n");
    for (int i=0;i<nActualInstructions;i++) {
        BasicBlock::PrintInstructionSimple(emitOrder[i],2,memContext);
    }
    printf("\n");
}

void RTLGenerator::DagEmitReloadX86(Dag *dag,int machineReg,int fpOffset) {
   EmitLoadBaseX86(machineReg,X86_EBP,fpOffset);
}

void RTLGenerator::DagEmitSpillX86(Dag *dag,int machineReg,int fpOffset) {
   EmitStoreBaseX86(machineReg,X86_EBP,fpOffset);
}

// returns true if the required operands should be pushed as they are visited
bool InstructionDagNode::PushInstructionOperands(Instruction *instruction) {
    return ((instruction->op!=Js::OpCode::CallI)&&
            (instruction->op!=Js::OpCode::NewScObject)&&
            (instruction->op!=Js::OpCode::NewScObjectSimple)&&
            (instruction->op!=Js::OpCode::Ld_A)&&
            (instruction->op!=Js::OpCode::TempArgOut_A));
}

// these operands are inline in the instruction and are pushed before explicit operands
void RTLGenerator::DagEmitPrefixOperands(Instruction *instruction) {
    switch (instruction->op) {
        case Js::OpCode::LdFld:
        case Js::OpCode::NewScFunc:
        case Js::OpCode::StFld:
        case Js::OpCode::StElemC:
        case Js::OpCode::LdElemC:
            PushImmediateX86(((IntConstInstruction*)instruction)->immInt);
            break;
    }
}

void InstructionDagNode::EmitRequirements(Dag *dag,int indentAmt,RTLGenerator *generator) {
    bool reqPrint=false;
    if (!ListFn<DagNode*>::Empty(constraints)) {
        if (!dag->silent) {
            Indent(indentAmt);
            printf("Requirements for instruction %d\n",instruction->ordinal);
        }
        reqPrint=true;
        if (!dag->silent) {
            Indent(indentAmt+2);
            printf("Constraints...\n");
        }
        for (List<DagNode*> *entry=constraints->next;!(entry->isHead);entry=entry->next) {
            dag->EmitNode(entry->data,indentAmt+4,generator,false);
        }
    }
    generator->DagEmitPrefixOperands(instruction);
    if (nOperands>0) {
        bool pushOperands=PushInstructionOperands(instruction);
        if (!reqPrint) {
            if (!dag->silent) {
                Indent(indentAmt);
                printf("Requirements for instruction %d\n",instruction->ordinal);
            }
        }
        if (!dag->silent) {
            Indent(indentAmt+2);
            printf("Operands...\n");
        }
        for (int i=nOperands-1;i>=0;i--) {
            dag->EmitNode(operands[i],indentAmt+4,generator,pushOperands);
            // assume node comes back in eax
        }
    }
}


void Dag::EmitCoreInstruction(Instruction *instruction,RTLGenerator *generator) {
    generator->EmitInstructionX86(instruction);
}

void RTLGenerator::EmitMovReg(int dest,int src) {
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,dest,src);
}

void Dag::LoadNodeIntoReg(DagNode *node,RTLGenerator *generator,int reg) {
    switch (node->GetNodeType()) {
        case DagNodeIntConst:
        case DagNodeDoubleConst: {
            generator->LoadConstantAtomX86(registerDescriptions[node->GetLabel()],reg);
            break;
        }
        case DagNodeLocation: {
            int label=node->GetLabel();
            RegisterDescription *registerDescription=registerDescriptions[label];
            int location=registerDescription->GetLocation();
            if (location!=NO_ASSIGNMENT) 
                generator->EmitMovReg(X86_EAX,location);
            else {
                generator->DagEmitReloadX86(this,X86_EAX,registerDescription->GetFramePointerOffset());
            }
            break;
        }
        case DagNodeInstruction: {
            // do nothing; will be in eax because instruction emitted as requirement
            break;
        }
    }
}

void InstructionDagNode::EnsureSingleOperandInEAX(Dag *dag,RTLGenerator *generator) {
    AssertMsg(nOperands==1,"Can only do this if single operand");
    DagNode *operandNode=operands[0];
    dag->LoadNodeIntoReg(operandNode,generator,X86_EAX);
}

void Dag::EmitInstruction(InstructionDagNode *idagNode,int indentAmt,RTLGenerator *generator) {
    Instruction *instruction=idagNode->GetInstruction();
    RegisterDescription *registerDescription=registerDescriptions[instruction->rd];
    int location=registerDescription->GetLocation();
    if (idagNode->GetEmitted()) {
        if (!silent) {
            Indent(indentAmt);
            printf("ref instruction %d",instruction->ordinal);
        }
        if (location!=NO_ASSIGNMENT) {
            if (!silent) {
                printf(" from machine register %s\n",RegName(location));
            }
            generator->EmitMovReg(X86_EAX,location);
        }
        else { 
            if (!silent) 
                printf(" from frame slot %d(ebp)\n",registerDescription->GetFramePointerOffset());
            generator->EmitLoadBaseX86(X86_EAX,X86_EBP,registerDescription->GetFramePointerOffset());
        }

    }
    else {
        idagNode->SetEmitted(true);
        idagNode->EmitRequirements(this,indentAmt,generator);
        if (!silent) {
            emitOrder[nActualInstructions++]=instruction;
            idagNode->Print(indentAmt,memContext);
        }
        bool pushOperands=InstructionDagNode::PushInstructionOperands(instruction);
        if (!pushOperands) {
            idagNode->EnsureSingleOperandInEAX(this,generator);
        }
        // emit the instrution itself; operands will have been pushed where appropriate
        EmitCoreInstruction(instruction,generator);
        // assume instruction returns in eax
        // TODO: only save to frame on last def if doesn't have multiple uses
        if (idagNode->HasMultipleUses()||
            ((location==NO_ASSIGNMENT)&&(registerDescription->GetRegisterType()==RegisterTypeFunctionScope))) {
                if (!silent)
                    Indent(indentAmt+2);
                if (location!=NO_ASSIGNMENT) {
                    if (!silent)
                        printf("  save to machine register %s\n",RegName(location));
                    generator->EmitMovReg(location,X86_EAX);
                }
                else {
                    generator->EmitStoreBaseX86(X86_EAX,X86_EBP,generator->AssignFrameSlotX86(registerDescription));
                    if (!silent)
                        printf("  save to frame slot %d(ebp)\n",registerDescription->GetFramePointerOffset());
                }
        }
    }
}

void RTLGenerator::EmitPushEAX() {
    *pbuf++ = I_PUSH_EAX;
}

void Dag::EmitNode(DagNode *node,int indentAmt,RTLGenerator *generator,bool pushNode) {
    switch (node->GetNodeType()) {
        case DagNodeIntConst:
        case DagNodeDoubleConst: {
            if (!silent)
                node->Print(indentAmt); 
            if (pushNode)
                generator->PushConstantAtomX86(registerDescriptions[node->GetLabel()]);
            break;
        }
        case DagNodeLocation: {
            int label=node->GetLabel();
            if (!silent)
                Indent(indentAmt);
            RegisterDescription *registerDescription=registerDescriptions[label];
            int location=registerDescription->GetLocation();
            if (!silent) {
                if (location!=NO_ASSIGNMENT) {
                    printf(" location %d from machine register %s\n",label,RegName(location));
                }
                else printf(" location %d from frame slot %d(ebp)\n",label,registerDescription->GetFramePointerOffset());
            }
            if (pushNode) {
                if (location!=NO_ASSIGNMENT) 
                    generator->PushOperandX86(label,location);
                else {
                    generator->DagEmitReloadX86(this,X86_EAX,registerDescription->GetFramePointerOffset());
                    generator->EmitPushEAX();
                }
            }
            break;
        }
        case DagNodeInstruction: {
            InstructionDagNode *opNode=(InstructionDagNode *)node;
            EmitInstruction(opNode,indentAmt,generator);
            // assume result of instruction is in eax
            if (pushNode) {
                generator->EmitPushEAX();
            }
            break;
        }
    }
}

void Dag::EmitBlock(BasicBlock *block,RTLGenerator *generator) {
    for (int i=0;i<nLocations;i++) {
        registerDescriptions[i]->NewBlock();
    }
    nEmitInstructions=block->NumberInstructions();
    // TODO: allocate this once per function and just reset for each block
    MachineRegisters *x86MachineRegisters=Anew(alloc,MachineRegisters,alloc,X86_NINTREGS,X86_TEMP_REGISTER,X86_EDX,X86_FRAME_POINTER,
        X86_STACK_POINTER,X86_CALLEE_SAVED);
    block->AllocateRegisters(x86MachineRegisters,registerDescriptions,nLocations);
    if (!silent) {
        emitOrder=(Instruction **)alloc->Alloc(nEmitInstructions*sizeof(Instruction *));
        nActualInstructions=0;
    }
    List<DagNode*> *q=ListFn<DagNode*>::MakeListHead(alloc);
    for (List<DagNode*> *entry=nodes->next;!(entry->isHead); entry=entry->next) {
        if (entry->data->IsRoot())
            ListFn<DagNode*>::Add(q,entry->data,alloc);
    }
    if (!silent)
        printf("Dag order for block at start offset 0x%x\n",block->GetStartOffset());
    // reload global registers
    for (int i=1;i<nLocations;i++) {
        RegisterDescription *registerDescription=registerDescriptions[i];
        if ((registerDescription->GetRegisterType()==RegisterTypeFunctionScope)&&
            (registerDescription->GetLocation()!=NO_ASSIGNMENT)&&
            (registerDescription->FirstAccessIsUse(block))) {
                if (!silent) {
                    printf("  reload %s = L%d at %d(ebp)\n",RegName(registerDescription->GetLocation()),i,
                           registerDescription->GetFramePointerOffset());
                }
                generator->DagEmitReloadX86(this,registerDescription->GetLocation(),registerDescription->GetFramePointerOffset());
        }
    }
    while (!(ListFn<DagNode*>::Empty(q))) {
        DagNode *node=ListFn<DagNode*>::PopEntry(q)->data;
        EmitNode(node,2,generator,false);
    }
    if ((blockEndNode!=null)&&(!blockEndNode->IsBranch())) {
        EmitNode(blockEndNode,2,generator,false);
    }
    for (int i=1;i<nLocations;i++) {
        RegisterDescription *registerDescription=registerDescriptions[i];
        if ((registerDescription->GetRegisterType()==RegisterTypeFunctionScope)&&
            (registerDescription->GetLocation()!=NO_ASSIGNMENT)&&
            (registerDescription->HasDefInBlock(block))) {
                generator->DagEmitSpillX86(this,registerDescription->GetLocation(),generator->AssignFrameSlotX86(registerDescription));
                if (!silent) {
                   printf("  L%d at %d(ebp) = %s\n",i,registerDescription->GetFramePointerOffset(),
                          RegName(registerDescription->GetLocation()));
                }

        }
    }
    if ((blockEndNode!=null)&&(blockEndNode->IsBranch())) {
        EmitNode(blockEndNode,2,generator,false);
    }
    if (!silent) {
        printf("with final sequence...\n");
        for (int i=0;i<nActualInstructions;i++) {
            BasicBlock::PrintInstructionSimple(emitOrder[i],2,memContext);
        }
    }
}

void RTLGenerator::PrintDags(ControlFlowGraph *cfg) {
    Dag *dag;
    BasicBlock *block;
    // set up frame slots
    firstLocalSlot = 0;
    curLocalSlot=firstLocalSlot;
    for (List<BasicBlock*> *blockEntry=cfg->GetBlocks()->next;!(blockEntry->isHead);blockEntry=blockEntry->next) {
        block=blockEntry->data;
        dag=Dag::BuildDag(block,nRegs,registerDescriptions,alloc,memContext,silent);
        dag->PrintEmitOrder(block,this);
    }
    block=cfg->GetExit();
    dag=Dag::BuildDag(block,nRegs,registerDescriptions,alloc,memContext,silent);
    dag->PrintEmitOrder(block,this);
}

void RTLGenerator::LoadOperandFromStack(int spOffset,int destReg) {
    *pbuf++ = I_LD;
    *pbuf++ = MakeModRM(MOD_DWORD_DISPLACEMENT,destReg,RM_IS_SIB);
    *pbuf++ = MakeSIB(SCALE_1,INDEX_IS_NONE,X86_ESP);
    *(int *)pbuf = spOffset;
    pbuf+=4;
}

void RTLGenerator::DagEmitCompareOpFastPathX86(BranchInstruction *branchInstruction,unsigned char **branchToHelper1Ref,
                                               unsigned char **branchPastHelperRef,int icondOp) {
    LoadOperandFromStack(0,X86_EDX);
    LoadOperandFromStack(4,X86_ECX);
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
    LoadOperandFromStack(4,X86_ECX);
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
    // pop helper operands since fast path succeeded
    AddOffsetX86(X86_ESP,8);
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::DagEmitConditionalBranchX86(BranchInstruction *branchInstruction,bool twoOperands) {
    bool hasFastPath=false;
    unsigned char *branchToHelper1=NULL;
    unsigned char *branchPastHelper=NULL;
    int icondOp;
    void *fn=NULL;
    switch (branchInstruction->op) {
        case Js::OpCode::BrFalse_A:
        case Js::OpCode::BrTrue_A:
            fn=Js::JavascriptConversion::ToBoolean;
            break;
        case Js::OpCode::BrOnEmpty:
            fn=Js::JavascriptOperators::OP_BrOnEmpty;
            break;
        case Js::OpCode::BrEq_A: 
            fn=Js::JavascriptOperators::Equal;
            icondOp=I_ICOND_EQ;
            //hasFastPath=true;
            //EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrNeq_A: 
            fn=Js::JavascriptOperators::NotEqual;
            icondOp=I_ICOND_NE;
            //hasFastPath=true;
            //EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrGt_A: 
            fn=Js::JavascriptOperators::Greater;
            icondOp=I_ICOND_GT;
            //hasFastPath=true;
            //EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrLt_A: 
            fn=Js::JavascriptOperators::Less;
            icondOp=I_ICOND_LT;
            //hasFastPath=true;
            //DagEmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrGe_A: 
            fn=Js::JavascriptOperators::GreaterEqual;
            icondOp=I_ICOND_GE;
            //hasFastPath=true;
            //EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        case Js::OpCode::BrLe_A: 
            fn=Js::JavascriptOperators::LessEqual;
            icondOp=I_ICOND_LE;
            //hasFastPath=true;
            //EmitCompareOpFastPathX86(branchInstruction,&branchToHelper1,&branchPastHelper,icondOp);
            break;
        default:
            AssertMsg(false,"Unimplemented conditional branch opcode");
            break;
     }
    if ((hasFastPath)&&(branchToHelper1!=NULL))
        *(unaligned int *)branchToHelper1=(int)(pbuf-(branchToHelper1+4));
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

void RTLGenerator::DagEmitAndAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelperRef,
                                             unsigned char **branchPastHelperRef) {
    LoadOperandFromStack(0,X86_EDX);
    LoadOperandFromStack(4,X86_ECX);
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
    // mov eax,ecx
    *pbuf++ = I_MOV_RR;
    *pbuf++ = MakeModRM(MOD_IS_REG,X86_EAX,X86_ECX);
    // pop helper operands since fast path succeeded
    AddOffsetX86(X86_ESP,8);
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}

void RTLGenerator::DagEmitAddAtomFastPathX86(Instruction *instruction,unsigned char **branchToHelper1Ref,
                                             unsigned char **branchToHelper2Ref,
                                             unsigned char **branchPastHelperRef) {
    LoadOperandFromStack(0,X86_EDX);
    LoadOperandFromStack(4,X86_ECX);
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
    // pop helper operands since fast path succeeded
    AddOffsetX86(X86_ESP,8);
    // jump past helper
    *pbuf++ = I_JMP_PCREL; 
    *branchPastHelperRef=pbuf;
    pbuf+=4;
}


void RTLGenerator::DagEmitRegisterOperation3X86(Instruction *instruction) {
    bool hasFastPath=false;
    unsigned char *branchToHelper1=NULL;
    unsigned char *branchToHelper2=NULL;
    unsigned char *branchPastHelper=NULL;

    // TODO: fast path
    switch (instruction->op) {
        case Js::OpCode::And_A: {
            //hasFastPath=true;
            //DagEmitAndAtomFastPathX86(instruction,&branchToHelper1,&branchPastHelper);
            break;
        }
        case Js::OpCode::Add_A: {
            //hasFastPath=true;
            //DagEmitAddAtomFastPathX86(instruction,&branchToHelper1,&branchToHelper2,&branchPastHelper);
            break;
            // these are faster with fast path off so far
        case Js::OpCode::Shl_A: {
            //hasFastPath=true;
            //EmitShlAtomFastPathX86(instruction,&branchToHelper1,&branchToHelper2,&branchPastHelper);
            break;
        }
        case Js::OpCode::Shr_A: {
            //hasFastPath=true;
            //EmitShrAtomFastPathX86(instruction,&branchToHelper1,&branchPastHelper);
            break;
        }
        }
    }
    if ((hasFastPath)&&(branchToHelper1!=NULL))
        *(unaligned int *)branchToHelper1=(int)(pbuf-(branchToHelper1+4));
    if ((hasFastPath)&&(branchToHelper2!=NULL))
        *(unaligned int *)branchToHelper2=(int)(pbuf-(branchToHelper2+4));
    // call the helper
    void *fn=NULL;
    switch (instruction->op) {
        case Js::OpCode::Add_A:
            fn=Js::JavascriptOperators::Add;
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
        default:
            AssertMsg(false,"Unimplemented register3 opcode");
            break;
    }
    // always in eax for now; later go right to assigned reg if is one
    FinishStdCall(X86_EAX,fn);
    if (hasFastPath)
        *(unaligned int *)branchPastHelper=(int)(pbuf-(branchPastHelper+4));
}

extern Js::Atom NewScObjectX86(Js::Atom function);

void RTLGenerator::DagEmitNewScObjectX86(CallInstruction *callInstruction) {
    unsigned char *branchToNullConstructor;
    unsigned char *branchExit1;
    unsigned char *branchExit2;
    void *fn=NewScObjectX86;

    // DAG:  assume dest is eax
    RegisterDescription *registerDescription=registerDescriptions[callInstruction->rs1];
    if ((registerDescription->GetRegisterType()==RegisterTypeObjectConstant)&&
        (registerDescription->GetConstDefOp()==Js::OpCode::LdC_A_Null)) {
        // just create new object (no constructor)
        PushImmediateX86(0);
        FinishStdCall(X86_EAX,fn);
    }
    else {
        // DAG:  assume fn is in eax
        *pbuf++ = I_CMP_I32;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I32_RVAL,X86_EAX);
        *(int *)pbuf=0;
        pbuf+=4;

        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_EQ;
        *(unaligned int *)pbuf=0;
        branchToNullConstructor=(unsigned char *)pbuf;
        pbuf+=4;

        // create frame slot to stash fn and allocated object
        curLocalSlot-=4;
        int offset=curLocalSlot;
        // stash eax (which is fn)
        *pbuf++ = I_ST;
        SetMovBaseOps(X86_EAX,X86_EBP,offset,SCALE_1);
        //  push function as argument
        *pbuf++ = I_PUSH_EAX;
        FinishStdCall(X86_EAX,fn);
        // destReg has new object; put into 'this' arg slot)
        MoveToArgSlot(X86_EAX,0);
        *pbuf++ = I_MOV_RR;
        // put allocated object into ecx
        *pbuf++ = MakeModRM(MOD_IS_REG,X86_ECX,X86_EAX);
        *pbuf++ = I_LD;
        //  load eax from stashed fnReg
        SetMovBaseOps(X86_EAX,X86_EBP,offset,SCALE_1);
        *pbuf++ = I_ST;
        //  store allocated object into frame slot
        SetMovBaseOps(X86_ECX,X86_EBP,offset,SCALE_1);
        DagEmitCallIX86(callInstruction);
        // FIX: TODO: pin this to this code object
        void *undefinedObject=Js::ScriptContext::Info(memContext)->GetUndefinedValueA();
        *pbuf++ = I_CMP_I32;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I32_RVAL,X86_EAX);
        *(unaligned int *)pbuf=(int)undefinedObject;
        pbuf+=4;
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NE;
        *(unaligned int *)pbuf=0;
        branchExit2=(unsigned char *)pbuf;
        pbuf+=4;
        // restore originally allocated object as return value (since return from constructor is undefined)
        *pbuf++ = I_LD;
        SetMovBaseOps(X86_EAX,X86_EBP,offset,SCALE_1);
        *pbuf++ = I_JMP_PCREL;
        *(unaligned int *)pbuf=0;
        branchExit1=pbuf;
        pbuf+=4;
        // branch target for big else
        *(int*)branchToNullConstructor=(unsigned int)(pbuf-(4+branchToNullConstructor));
        PushImmediateX86(0);
        FinishStdCall(X86_EAX,fn);
        // exit from this big gizmo
        *(int*)branchExit1=(unsigned int)(pbuf-(4+branchExit1));
        *(int*)branchExit2=(unsigned int)(pbuf-(4+branchExit2));
    }
}

// arguments have been pushed right-to-left; call target and clean up stack
void RTLGenerator::DagEmitCallIX86(CallInstruction *callInstruction) {
    //  load native entry point from script function into eax
    int epOffset=offsetof(Js::JavascriptFunction,NativeEntryPoint);
    *pbuf++ = I_LD;
    // DAG: assume leftmost operand is in eax (it is the function)
    SetMovBaseOps(X86_EDX,X86_EAX,epOffset,SCALE_1);
    // push number of arguments (including 'this')
    PushImmediateX86(callInstruction->nArgs);
    // push function wrapper
    *pbuf++ = I_PUSH_EAX;
    // call native entry point
    *pbuf++ = I_ICALL;
    *pbuf++ = MakeModRM(MOD_IS_REG,I_ICALL_RVAL,X86_EDX);
    // DAG: for now just assume call always returns into eax
    // the save def logic will put eax into register or frame slot as necessary
    // clean up (FIX: need way of not doing this if stdcall;
    // for example, could determine at call site whether stdcall)
    // CHECK: make sure we are popping any extra args
    int offset=(callInstruction->nArgs+2)*sizeof(int);
    AddOffsetX86(X86_ESP,offset);
}

void RTLGenerator::DagEmitRegisterOperation2X86(Instruction *instruction) {
    // call the helper
    void *fn=NULL;
    switch (instruction->op) {
        case Js::OpCode::Neg_A:
            fn=Js::JavascriptOperators::Negate;
            break;
        case Js::OpCode::Not_A:
            fn=Js::JavascriptOperators::Not;
            break;
        case Js::OpCode::LdLen_A:
            fn=Js::JavascriptOperators::GetLength;
            break;
        case Js::OpCode::NewArray:
            fn=Js::FixedSizeArray::OP_NewArray;
            break;
        case Js::OpCode::NewScArray:
            fn=Js::JavascriptArray::OP_NewScArray;
            break;
        case Js::OpCode::GetEnumerator:
            fn=Js::JavascriptOperators::OP_GetEnumerator;
            break;
        case Js::OpCode::GetCurrent:
            fn=Js::JavascriptOperators::OP_GetCurrent;
            break;
        default:
            AssertMsg(false,"Unimplemented register2 opcode");
            break;
    }
    FinishStdCall(X86_EAX,fn);
}

// src is in eax; move to reg or frame slot

void RTLGenerator::DagEmitLoadOperandX86(int destReg) {
    RegisterDescription *registerDescription=registerDescriptions[destReg];
    int location=registerDescription->GetLocation();
    if (location!=NO_ASSIGNMENT) {
        if (!silent)
            printf("  move eax to machine register %s\n",RegName(location));
        EmitMovReg(location,X86_EAX);
    }
    else {
        EmitStoreBaseX86(X86_EAX,X86_EBP,AssignFrameSlotX86(registerDescription));
        if (!silent)
            printf("  save eax to frame slot %d(ebp)\n",registerDescription->GetFramePointerOffset());
    }
}

// TODO: use undefined instead of null
void RTLGenerator::DagEmitArgInX86(ArgInstruction *argInstruction) {
   unsigned char *branchToLoadNull;
   unsigned char *branchToExit;
   int machineReg=X86_EAX;
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

// assumes arg out instructions have been ordered right to left (high to low index)
// assumes operand is in eax (even if it's a constant)
void RTLGenerator::DagEmitArgOutX86(ArgInstruction *argInstruction) {
    AssertMsg(argInstruction->op==Js::OpCode::TempArgOut_A,"Only output args using TempArgOut_A opcode");
    MoveToArgSlot(X86_EAX,argInstruction->argIndex*4);
}

void RTLGenerator::DagEmitLoadPropertyX86(IntConstInstruction *instruction) {
    void *fn;
#define INLINE_CACHE
#ifdef INLINE_CACHE
    if (instruction->op==Js::OpCode::LdFld) {
        unsigned char *branchToSlowPath1;
        unsigned char *branchToSlowPath2;
        unsigned char *branchAroundSlowPath;
        // four arguments *type,*slotIndex,instance, fieldId
        LoadPatch *loadPatch=MakeStruct(alloc,LoadPatch);
        loadPatch->type=(void*)LoadPatchInitialType;  // this can not be a type pointer
        loadPatch->slotIndex=(-1);
        // check that lhs is an object
        LoadOperandFromStack(0,X86_EDX); // instance
        EmitMovReg(X86_ECX,X86_EDX);     // squirrel instance into ecx
        // and edx,0x1
        *pbuf++ = I_AND_MI8;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_AND_MI8_RVAL,X86_EDX);
        *pbuf++ = 0x1;
        // go to helper call if is a tagged int
        *pbuf++ = I_CMP_I8;
        *pbuf++ = MakeModRM(MOD_IS_REG,I_CMP_I8_RVAL,X86_EDX);
        *pbuf++ = 0x1;
        // branch
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_EQ;
        branchToSlowPath1=pbuf;
        pbuf+=4;
        // load type into edx (instance is in ecx)
        EmitLoadBaseX86(X86_EDX,X86_ECX,offsetof(Js::RecyclableObject, type));
        *pbuf++ = I_CMP_RM;
        SetMovBaseOps(X86_EDX,X86_REG_NONE,(int)&(loadPatch->type),SCALE_1);
        *pbuf++ = I_JCC1;
        *pbuf++ = I_ICOND_NE;
        branchToSlowPath2=pbuf;
        pbuf+=4;
        // fast path
        // load slot pointer into edx
        EmitLoadBaseX86(X86_EDX,X86_ECX,offsetof(Js::DynamicObject,m_pSlots));
        *pbuf++ = I_LD;
        // load cached slot index into ecx
        SetMovBaseOps(X86_ECX,X86_REG_NONE,(int)&(loadPatch->slotIndex),SCALE_1);
        // mov mrd,[edx+4*ecx]
        *pbuf++ = I_LD;
        *pbuf++ = MakeModRM(MOD_NO_DISP_OR_DIRECT,X86_EAX,RM_IS_SIB);
        *pbuf++ = MakeSIB(SCALE_4,X86_ECX,X86_EDX);
        // pop helper operands since fast path succeeded
        AddOffsetX86(X86_ESP,8);
        // jump to exit
        *pbuf++ = I_JMP_PCREL; 
        branchAroundSlowPath=pbuf;
        pbuf+=4;
        // slow path
        *(int*)branchToSlowPath1=(unsigned int)(pbuf-(4+branchToSlowPath1));
        *(int*)branchToSlowPath2=(unsigned int)(pbuf-(4+branchToSlowPath2));
        // helper args already pushed (including field index)
        PushImmediateX86((int)&(loadPatch->slotIndex));
        PushImmediateX86((int)&(loadPatch->type));
        fn=Js::DynamicObject::PatchGetValue;
        FinishStdCall(X86_EAX,fn);
        // exit
        *(int*)branchAroundSlowPath=(unsigned int)(pbuf-(4+branchAroundSlowPath));
    }
    else {
        if (instruction->op==Js::OpCode::NewScFunc)
            fn=Js::JavascriptFunction::OP_NewScFunc;
        else fn=Js::FixedSizeArray::OP_LdElemC;
        // call the helper
        FinishStdCall(X86_EAX,fn);
    }
#else
    if (instruction->op==Js::OpCode::LdFld) 
        fn=Js::JavascriptOperators::GetProperty;
    else if (instruction->op==Js::OpCode::NewScFunc)
        fn=Js::JavascriptFunction::OP_NewScFunc;
    else fn=Js::FixedSizeArray::OP_LdElemC;
    // call the helper
    FinishStdCall(X86_EAX,fn);
#endif
}

// re-arrange the arguments
void LocalSetProperty(Js::Atom aInstance,Js::Atom aNewValue,int propertyId) {
    Js::JavascriptOperators::SetProperty(aInstance,propertyId,aNewValue);
}

// re-arrange the arguments
void LocalStElemC(Js::Atom aInstance,Js::Atom aNewValue,int propertyId) {
    Js::FixedSizeArray::OP_StElemC(aInstance,propertyId,aNewValue);
}

// TODO: inline caching
void RTLGenerator::DagEmitStorePropertyX86(IntConstInstruction *instruction) {
    // call the helper
    void *fn;
    if (instruction->op==Js::OpCode::StFld)
        fn=LocalSetProperty;
    else fn=LocalStElemC;
    FinishStdCall(X86_EAX,fn);
}

// TODO: inline caching
void RTLGenerator::DagEmitLoadElementI(IntConstInstruction *instruction) {
    void *fn=Js::JavascriptOperators::GetElementI;
    FinishStdCall(X86_EAX,fn);
}

// TODO: inline caching
void RTLGenerator::DagEmitStoreElementI(StElemInstruction *instruction) {
    // call the helper
    void *fn=Js::JavascriptOperators::SetElementI;
    FinishStdCall(X86_EAX,fn);
}

void RTLGenerator::EmitInstructionX86(Instruction *instruction) {
    Js::OpCode op=instruction->op;
    switch (op) {
        case Js::OpCode::FunctionExit: 
           EmitEpilogueX86(curLocalSlot);
           break;
        case Js::OpCode::Nop: 
            break;
        case Js::OpCode::Ret: 
            EmitUnconditionalBranchX86((BranchInstruction *)instruction);
            break;
        case Js::OpCode::BrEq_A: 
        case Js::OpCode::BrNeq_A: 
        case Js::OpCode::BrGt_A: 
        case Js::OpCode::BrLt_A: 
        case Js::OpCode::BrGe_A: 
        case Js::OpCode::BrLe_A: 
            DagEmitConditionalBranchX86((BranchInstruction*)instruction,true);
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
            DagEmitRegisterOperation3X86(instruction);
            break;
        case Js::OpCode::CallI:
            DagEmitCallIX86((CallInstruction*)instruction);
            break;
        case Js::OpCode::NewScObject:
        case Js::OpCode::NewScObjectSimple:
            DagEmitNewScObjectX86((CallInstruction*)instruction);
            break;
        case Js::OpCode::Ld_A:
            DagEmitLoadOperandX86(instruction->rd);
            break;
        case Js::OpCode::Neg_A:
        case Js::OpCode::Not_A:
        case Js::OpCode::LdLen_A:
        case Js::OpCode::NewArray:
        case Js::OpCode::NewScArray:
        case Js::OpCode::GetEnumerator:
        case Js::OpCode::GetCurrent:
            DagEmitRegisterOperation2X86(instruction);
            break;
        case Js::OpCode::Br:
            EmitUnconditionalBranchX86((BranchInstruction *)instruction);
            break;
        case Js::OpCode::LdRoot:
            EmitLoadRootX86(NULL,X86_EAX);
            break;
        case Js::OpCode::LdUndef:
            EmitLoadUndefX86(X86_EAX);
            break;
        case Js::OpCode::LdTrue:
        case Js::OpCode::LdFalse:
            LoadConstantAtomX86(registerDescriptions[instruction->rd],X86_EAX);
            break;
        case Js::OpCode::LdC_A_Null:
        case Js::OpCode::LdC_A_I4_0:
        case Js::OpCode::LdC_A_I4_1:
        case Js::OpCode::LdC_A_I4:
        case Js::OpCode::LdC_A_R8:
        case Js::OpCode::LdC_A_R8_0:
            // these are emitted on use as an operand
            break;
        case Js::OpCode::BrFalse_A:
        case Js::OpCode::BrTrue_A:
        case Js::OpCode::BrOnEmpty:
            DagEmitConditionalBranchX86((BranchInstruction*)instruction,false);
            break;
        case Js::OpCode::ArgIn_A:
            DagEmitArgInX86((ArgInstruction*)instruction);
            break;
        case Js::OpCode::TempArgOut_A:
            DagEmitArgOutX86((ArgInstruction*)instruction);
            break;
        case Js::OpCode::ArgOut_A:
            // Using TempArgOut instead
            //EmitArgOutX86((ArgInstruction*)instruction);
            break;
        case Js::OpCode::LdFld:
            DagEmitLoadPropertyX86((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::NewScFunc:
            DagEmitLoadPropertyX86((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::StFld:
            DagEmitStorePropertyX86((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::LdElemC:
            DagEmitLoadPropertyX86((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::StElemC:
            DagEmitStorePropertyX86((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::LdElemI_A:
            DagEmitLoadElementI((IntConstInstruction*)instruction);
            break;
        case Js::OpCode::StElemI_A:
            DagEmitStoreElementI((StElemInstruction*)instruction);
            break;
        case Js::OpCode::LdStr: {
            instruction->mrd=X86_EAX;
            EmitLdStr((LdStrInstruction *)instruction);
            break;
        }
        case Js::OpCode::NewRegEx: {
            instruction->mrd=X86_EAX;
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
            instruction->mrd=X86_EAX;
            EmitLdEnv(instruction);
            break;
        default: {
            AssertMsg(false,"Unimplemented opcode");
            break;
        }
    }
    AssertMsg((pbuf-codeBuffer)<(int)codeBufferSize,"code buffer overflow");
}

extern bool traceBB;

// Generates buffer of X86 code using dag for each BB
void RTLGenerator::EmitFunctionUsingDagsX86(ControlFlowGraph *cfg,int nRegs) {
    firstLocalSlot = 0;
    curLocalSlot=firstLocalSlot;
    nativeRelocs=ListFn<NativeReloc*>::MakeListHead(alloc);
    // allocate a big buffer and then copy and trim at end
    codeBufferSize=(30*instructionCount)+200;
    codeBuffer=(unsigned char *)calloc(codeBufferSize,1);
    pbuf=codeBuffer;
    ListFn<BasicBlock*>::Add(cfg->GetBlocks(),cfg->GetExit(),alloc);
    EmitPrologueX86();
    for (List<BasicBlock*> *blockEntry=cfg->GetBlocks()->next;!(blockEntry->isHead);blockEntry=blockEntry->next) {
        blockEntry->data->SetNativeStartOffset((unsigned int)(pbuf-codeBuffer));
        BasicBlock* block=blockEntry->data;
        if (traceBB) {
            EmitTraceBBX86(block);
        }
        // FIX: TODO: use shorter-lifetime allocator for dag state
        Dag *dag=Dag::BuildDag(block,nRegs,registerDescriptions,alloc,memContext,silent);
        dag->EmitBlock(block,this);
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
