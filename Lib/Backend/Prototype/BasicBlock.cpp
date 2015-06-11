/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

void BasicBlock::PrintInstruction(Instruction *instruction,MemoryContext* memContext) {
    printf("  Instruction op %-10s rd:%d->%d rs1:%d->%d rs2:%d->%d",Js::OpCodeNames[instruction->op],
        instruction->rd,instruction->mrd,instruction->rs1,instruction->mrs1,instruction->rs2,instruction->mrs2);
    switch (instruction->op) {
        case Js::OpCode::TempArgOut_A:
            printf(" arg index %d\n",((ArgInstruction*)instruction)->argIndex);
            break;
        case Js::OpCode::LdFld:
        case Js::OpCode::StFld: {
            int propertyId=((IntConstInstruction*)instruction)->immInt;
            wprintf(L" field %s\n",Js::ScriptContext::Info(memContext)->GetPropertyName(propertyId)->GetBuffer());
            break;
        }
        case Js::OpCode::Ret: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            printf("    Return jumps to block at offset %x native offset %x\n",branchInstruction->branchTarget->GetStartOffset(),
                branchInstruction->branchTarget->GetNativeStartOffset());
            break;
                              }
        case Js::OpCode::BrEq_A: 
        case Js::OpCode::BrNeq_A: 
        case Js::OpCode::BrGt_A: 
        case Js::OpCode::BrLt_A: 
        case Js::OpCode::BrGe_A: 
        case Js::OpCode::BrLe_A: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            printf("    Conditional branch jumps to block at offset %x\n",branchInstruction->branchTarget->GetStartOffset());
            break;
                                 }
        case Js::OpCode::BrOnHasProperty:
        case Js::OpCode::BrOnNoProperty: {
            BranchImmInstruction *branchInstruction=(BranchImmInstruction*)instruction;
            int propertyId=branchInstruction->immInt;
            wprintf(L" field %s\n",Js::ScriptContext::Info(memContext)->GetPropertyName(propertyId)->GetBuffer());
            printf("    Conditional branch jumps to block at offset %x\n",branchInstruction->branchTarget->GetStartOffset());
            break;
                                     }
        case Js::OpCode::Br: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            printf("    Unconditional branch jumps to block at offset %x\n",branchInstruction->branchTarget->GetStartOffset());
            break;
        }
        default:
            printf("\n");
            break;
    }

}

void BasicBlock::PrintInstructionSimple(Instruction *instruction,int indentAmt,MemoryContext* memContext) {
    Indent(indentAmt);
    if (instruction->rd!=RTLGenerator::ReturnRegister) 
        printf("  Instruction %d: L%d=op %-10s",instruction->ordinal,instruction->rd,Js::OpCodeNames[instruction->op]);
    else printf("  Instruction %d: op %-10s",instruction->ordinal,Js::OpCodeNames[instruction->op]);
    switch (instruction->op) {
        case Js::OpCode::LdStr:
            wprintf(L" string %s\n",((Js::LiteralString*)((LdStrInstruction *)instruction)->str)->GetSz());
            break;
        case Js::OpCode::TempArgOut_A:
            printf(" arg index %d\n",((ArgInstruction*)instruction)->argIndex);
            break;
        case Js::OpCode::ArgIn_A:
            printf(" arg index %d\n",((ArgInstruction*)instruction)->argIndex);
            break;
        case Js::OpCode::LdFld:
        case Js::OpCode::StFld: {
            int propertyId=((IntConstInstruction*)instruction)->immInt;
            wprintf(L" field %s\n",Js::ScriptContext::Info(memContext)->GetPropertyName(propertyId)->GetBuffer());
            break;
        }
        case Js::OpCode::Ret: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            Indent(indentAmt);
            printf("    Return jumps to block at offset %x native offset %x\n",branchInstruction->branchTarget->GetStartOffset(),
                branchInstruction->branchTarget->GetNativeStartOffset());
            break;
        }
        case Js::OpCode::BrEq_A: 
        case Js::OpCode::BrNeq_A: 
        case Js::OpCode::BrGt_A: 
        case Js::OpCode::BrLt_A: 
        case Js::OpCode::BrGe_A: 
        case Js::OpCode::BrLe_A: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            Indent(indentAmt);
            printf("    Conditional branch jumps to block at offset %x\n",branchInstruction->branchTarget->GetStartOffset());
            break;
                                 }
        case Js::OpCode::Br: {
            BranchInstruction *branchInstruction=(BranchInstruction*)instruction;
            printf("\n");
            Indent(indentAmt);
            printf("    Unconditional branch jumps to block at offset %x\n",branchInstruction->branchTarget->GetStartOffset());
            break;
        }
        default:
            printf("\n");
            break;
    }

}

void BasicBlock::AllocateRegisters(MachineRegisters *machineRegisters,RegisterDescription **registerDescriptions,int nLocations) {
    for (int i=1;i<nLocations;i++) {
        RegisterDescription *registerDescription=registerDescriptions[i];
        RegisterType regType=registerDescription->GetRegisterType();
        if ((regType==RegisterTypeBlockScope)||(regType==RegisterTypeFunctionScope)) {
            int nAccesses=registerDescription->GetNumberOfAccesses(this);
            if (nAccesses>0) {
                MachineRegister *machineReg=machineRegisters->GetFreeVarReg();
                if (machineReg==NULL) {
                    // compete for register
                    machineReg=machineRegisters->CompeteForRegister(nAccesses,this,registerDescriptions);
                }
                if (machineReg!=NULL) {
                    //printf("assigned %d with %d accesses to %s\n",i,nAccesses,RegName(machineReg->index));
                    registerDescription->SetLocation(machineReg->index);
                    machineReg->currentAssignment=i;
                }
            }
        }
    }
}

int BasicBlock::NumberInstructions() {
    int count=0;
    for (List<Instruction*> *entry=instructions->next;!(entry->isHead);entry=entry->next) {
        entry->data->ordinal=count++;
    }
    return count;
}

void BasicBlock::Print(MemoryContext* memContext) {
    printf("\nBasic Block at offset %x nat %x; instructions...\n",startOffset,nativeStartOffset);
    for (List<Instruction*> *entry=instructions->next;!(entry->isHead);entry=entry->next) {
        PrintInstruction(entry->data,memContext);
    }
}

BasicBlock *BasicBlock::SplitIfNecessary(Instruction *instruction,int targetOffset) {
    if ((targetOffset==startOffset)||(ListFn<Instruction*>::Empty(instructions))||(instructions->next->data==instruction)) {
        return this;
    }

    List<Instruction*> *entry;
    for (entry=instructions->next;!(entry->isHead); entry=entry->next) {
        if (entry->data==instruction)
            break;
    }
    if (!(entry->isHead)) {
        BasicBlock *splitBlock=Anew(alloc,BasicBlock,alloc,targetOffset);
        // transfer rest of instructions to new basic block
        while (!(entry->isHead)) {
            List<Instruction*> *next;
            next=entry->next;
            splitBlock->AddInstruction(entry->data);
            ListFn<Instruction*>::RemoveEntry(entry);
            entry=next;
        }
        return splitBlock;
    }
    else return this;
}

