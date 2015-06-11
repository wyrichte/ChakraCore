/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

bool MachineRegisters::IsUsed(Instruction *useInstruction,int sourceReg) {
    if (useInstruction==NULL)
        return false;
    if (useInstruction->rs1==sourceReg) {
        return true;
    }
    if (useInstruction->rs2==sourceReg) {
        return true;
    }
    if ((useInstruction->op==Js::OpCode::StElemI_A)&&
        ((((StElemInstruction *)useInstruction)->val)==sourceReg))
        return true;
    return false;
}

MachineRegister *MachineRegisters::FindSpillCandidate(Instruction *useInstruction,RegisterDescription **registerDescriptions) {
    MachineRegister *best=NULL;
    for (int i=0;i<nIntRegs;i++) {
        MachineRegister *machineRegister=intRegisters[(i+robin)%nIntRegs];
        if (!(machineRegister->isScratch||machineRegister->isTemp||machineRegister->isFramePointer||machineRegister->isStackPointer)) {
            if ((machineRegister->currentAssignment!=NO_ASSIGNMENT)&&(machineRegister->isCalleeSaved)&&
                (!(IsUsed(useInstruction,machineRegister->currentAssignment)))) {
                    if (best==NULL) {
                        best=machineRegister;
                        //robin++;
                        //return best;
                    }
                    else {
                        RegisterDescription *bestDescription=registerDescriptions[best->currentAssignment];
                        RegisterDescription *candidateDescription=registerDescriptions[machineRegister->currentAssignment];
                        if (candidateDescription->NextAccessOrdinal()<bestDescription->NextAccessOrdinal()) {
                            best=machineRegister;
                        }
                        /*
                        if (candidateDescription->NRemainingAccesses()>bestDescription->NRemainingAccesses()) {
                            best=machineRegister;
                        }
                        */

                    }
            }
        }
    }
    if (best!=NULL)
        robin++;
    return best;
}

// assume no free registers
MachineRegister *MachineRegisters::CompeteForRegister(int nAccesses,BasicBlock *block,RegisterDescription **registerDescriptions) {
    for (int i=0;i<nIntRegs;i++) {
        MachineRegister *candidate=intRegisters[i];
        if (!(candidate->isScratch||candidate->isTemp||candidate->isFramePointer||candidate->isStackPointer)) {
            if (candidate->isCalleeSaved) {
                AssertMsg(candidate->currentAssignment!=NO_ASSIGNMENT,"should only call CompeteForRegister when no free registers");
                RegisterDescription *currentDescription=registerDescriptions[candidate->currentAssignment];
                if (currentDescription->GetNumberOfAccesses(block)<nAccesses) {
                    currentDescription->ClearLocation();
                    return candidate;
                }
            }
        }
    }
    return NULL;
}

MachineRegister *MachineRegisters::GetFreeVarReg() {
    MachineRegister *reg=NULL;
    for (int i=0;i<nIntRegs;i++) {
        MachineRegister *candidate=intRegisters[i];
        if (candidate->isCalleeSaved) {
            if (freeRegs&(1<<i)) {
                freeRegs&=(~(1<<i));
                reg=candidate;
                break;
            }
        }
    }
    return reg;
}

void MachineRegisters::HomeAndFreeAll(bool insertBefore,BasicBlock *block,List<Instruction*> *instructionEntry,RegisterDescription **registerFile){ 
    for (int i=1;i<nIntRegs;i++) {
        MachineRegister *reg=intRegisters[i];
        if (!(reg->isTemp||reg->isFramePointer||reg->isStackPointer)) {
            if (reg->currentAssignment!=NO_ASSIGNMENT) {
                RegisterDescription *registerDescription=registerFile[reg->currentAssignment];
                if ((registerDescription->GetRegisterType()==RegisterTypeFunctionScope)&&registerDescription->HasDefInBlock(block)) {
                    Instruction *spillInstruction=MakeStructZ(alloc,Instruction);
                    spillInstruction->op=Js::OpCode::Spill;
                    spillInstruction->mrs1= reg->index;               // machine register
                    spillInstruction->rs2=*(Js::RegSlot*)&(reg->currentAssignment);     // byte code register
                    if (insertBefore) {
                        List<Instruction*> *spillEntry=ListFn<Instruction*>::MakeListEntry(alloc);
                        spillEntry->data=spillInstruction;
                        ListFn<Instruction*>::InsertBefore(instructionEntry,spillEntry);
                    }
                    else ListFn<Instruction*>::Add(block->GetInstructions(),spillInstruction,alloc);
                }
            }
            FreeReg(i);
        }
    }
    robin=1;
}
