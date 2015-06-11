/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

bool RegisterDescription::HasDefInBlock(BasicBlock *block) {
    for (List<Instruction*> *entry=accesses->next;!(entry->isHead);entry=entry->next) {
        if ((entry->data->basicBlock==block)&&(IsDef(entry->data))&&
            (!IsArgIn(entry->data))) {
                return true;
        }
    }
    return false;
}

bool RegisterDescription::FirstAccessIsUse(BasicBlock *block) {
    for (List<Instruction*> *entry=accesses->next;!(entry->isHead);entry=entry->next) {
        if (entry->data->basicBlock==block) {
            return(!IsDef(entry->data));
        }
    }
    return false;
}

Instruction *RegisterDescription::GetNextUse(Instruction *instruction) {
        bool found=false;
        for (List<Instruction*> *entry=accesses->next;!(entry->isHead);entry=entry->next) {
            if (!found) {
                if (entry->data==instruction) {
                    found=true;
                }
            }
            else {
                if (!IsDef(entry->data)) {
                    return(entry->data);
                }
            }
        }
        return NULL;
    }

MachineRegister *RegisterDescription::CreateFreeReg(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
        Instruction *useInstruction) {
        Instruction *spillInstruction=MakeStructZ(alloc,Instruction);
        MachineRegister *machineReg=machineRegisters->FindSpillCandidate(useInstruction,registerFile);
        AssertMsg(machineReg!=NULL,"Null spill candidate");
        spillInstruction->op=Js::OpCode::Spill;
        spillInstruction->mrs1= machineReg->index;   // machine register
        spillInstruction->rs2=(Js::RegSlot)machineReg->currentAssignment;  // byte code register
        *prefix=spillInstruction;
        RegisterDescription *spilledRegDescr=registerFile[machineReg->currentAssignment];
        spilledRegDescr->ClearLocation();
        return machineReg;
    }

    MachineRegister *RegisterDescription::GetVarRegister(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
        Instruction *useInstruction) {
        MachineRegister *freeVarReg=machineRegisters->GetFreeVarReg();
        if (freeVarReg==NULL) {
            MachineRegister *machineReg=CreateFreeReg(machineRegisters,registerFile,prefix,useInstruction);
            return machineReg;
        }
        else {
            return freeVarReg;
        }
    }

    MachineRegister *RegisterDescription::Reload(MachineRegisters *machineRegisters,RegisterDescription **registerFile,Instruction **prefix,
        Instruction *useInstruction) {
        MachineRegister *freeVarReg=machineRegisters->GetFreeVarReg();
        if (freeVarReg==NULL) {
            Instruction *exchangeInstruction=MakeStructZ(alloc,Instruction);
            MachineRegister *machineReg=machineRegisters->FindSpillCandidate(useInstruction,registerFile);
            AssertMsg(machineReg!=NULL,"Null spill candidate");
            exchangeInstruction->op=Js::OpCode::ExchangeLocations;
            exchangeInstruction->mrd=machineReg->index;  // machine register
            exchangeInstruction->rs1=(Js::RegSlot)machineReg->currentAssignment;  // byte code register to spill to
            exchangeInstruction->rs2=index;       // byte code register to reload from
            *prefix=exchangeInstruction;
            RegisterDescription *spilledRegDescr=registerFile[machineReg->currentAssignment];
            spilledRegDescr->ClearLocation();
            return machineReg;
        }
        else {
            Instruction *reloadInstruction=MakeStructZ(alloc,Instruction);
            reloadInstruction->op=Js::OpCode::Reload;
            reloadInstruction->mrd=freeVarReg->index;  // machine register
            reloadInstruction->rs1=index;   // byte code register
            *prefix=reloadInstruction;
            return freeVarReg;
        }
    }

    MachineRegister *RegisterDescription::GetTempRegister(Instruction *defInstruction,MachineRegisters *machineRegisters,
        RegisterDescription **registerFile,Instruction **prefix,Instruction *useInstruction) {
            MachineRegister *freeTempReg=machineRegisters->GetFreeVarReg();
            if (freeTempReg==NULL) {
                return CreateFreeReg(machineRegisters,registerFile,prefix,useInstruction);
            }
            else return freeTempReg;
    }

    void RegisterDescription::FreeIfNotUsed(MachineRegisters *machineRegisters,Instruction *curInstruction) {
        if (regType==RegisterTypeBlockScope) {
            Instruction *nextUse=GetNextUse(curInstruction);
            if (nextUse==NULL) {
                machineRegisters->FreeReg(location);
                location=NO_ASSIGNMENT;
            }
        }
    }

    int RegisterDescription::AssignSourceRegister(List<Instruction*> *useInstructionEntry,Instruction *useInstruction,MachineRegisters *machineRegisters,RegisterDescription **registerFile,
                                                  Instruction **prefix) {
            if (index==0)
                return 0;
            lastAccessEntry=useInstructionEntry;
        switch (regType) {
            case RegisterTypeIntConstant:       
                // holds an integer constant
            case RegisterTypeDoubleConstant:   
                // holds a double constant
            case RegisterTypeObjectConstant:    
                // holds a constant object (such as undefined or global)
                return index; // for now, assume we can synthesize this constant locally
            case RegisterTypeBlockScope:  {
                // variable or temporary used only within a single block
                if (location==NO_ASSIGNMENT) {
                    MachineRegister *machineRegister=Reload(machineRegisters,registerFile,prefix,useInstruction);
                    location=machineRegister->index;
                    machineRegister->currentAssignment=index;
                }
                return location;
            }
            case RegisterTypeFunctionScope:      
                // variable or argument used across blocks
                if (location==NO_ASSIGNMENT) {
                    MachineRegister *machineRegister=Reload(machineRegisters,registerFile,prefix,useInstruction);
                    location=machineRegister->index;
                    machineRegister->currentAssignment=index;
                }
                return location;
            case RegisterTypeUnknown: 
            default:
                AssertMsg(false,"Unknown register type in AssignSourceRegister");
                return 0;
        }
    }

    bool RegisterDescription::IsArgOutDef(Instruction *defInstruction) {
        if ((regType==RegisterTypeBlockScope)&&(ListFn<Instruction*>::Count(accesses)==2)) {
            Instruction *useInstruction=GetNextUse(defInstruction);
            if (useInstruction!=NULL) {
                return useInstruction->op==Js::OpCode::TempArgOut_A;
            }
        }
        return false;
    }

    bool HasObjectConstantSource(Instruction *useInstruction,RegisterDescription **registerFile) {
        if (useInstruction->rs1!=RTLGenerator::ReturnRegister) {
            RegisterDescription *registerDescription=registerFile[useInstruction->rs1];
            if (registerDescription->GetRegisterType()==RegisterTypeObjectConstant)
                return true;
        }
        if (useInstruction->rs2!=RTLGenerator::ReturnRegister) {
            RegisterDescription *registerDescription=registerFile[useInstruction->rs2];
            if (registerDescription->GetRegisterType()==RegisterTypeObjectConstant)
                return true;
        }
        if (useInstruction->op==Js::OpCode::StElemI_A) {
            StElemInstruction *stElemInstruction=(StElemInstruction*)useInstruction;
            if (stElemInstruction->val!=RTLGenerator::ReturnRegister) {
                RegisterDescription *registerDescription=registerFile[stElemInstruction->val];
                if (registerDescription->GetRegisterType()==RegisterTypeObjectConstant)
                    return true;
            }
        }
        return false;
    }

    bool IsValidTempPair(Instruction *defInstruction,Instruction *useInstruction,RegisterDescription **registerFile) {
        if ((useInstruction->op==Js::OpCode::NewScObject)||
            (useInstruction->op==Js::OpCode::CallI)||
            (defInstruction->op==Js::OpCode::NewScObject)||
            (HasObjectConstantSource(useInstruction,registerFile)))
            return false;
        else return true;
    }

    bool RegisterDescription::IsTemporaryDef(List<Instruction*> *entry,Instruction *defInstruction,
        RegisterDescription **registerFile) {
        if (ListFn<Instruction*>::Count(accesses)>2)
            return false;
        Instruction *useInstruction=GetNextUse(defInstruction);
        if (useInstruction!=NULL) {
            if ((entry->next->data==useInstruction)&&IsValidTempPair(defInstruction,useInstruction,registerFile)) {
//                    BasicBlock::PrintInstructionSimple(defInstruction);
                    return true;
            }
            else return false;
        }
        else {
//            printf("no next use");
//            BasicBlock::PrintInstructionSimple(defInstruction);
            return true;
        }
    }


    int RegisterDescription::NRemainingAccesses() {
        if ((lastAccessEntry==NULL)||(lastAccessEntry->next->isHead)) {
            return 0;
        }
        else {
            BasicBlock *block=lastAccessEntry->data->basicBlock;
            int count=0;
            for (List<Instruction*> *entry=lastAccessEntry->next;!(entry->isHead||(entry->data->basicBlock!=block));
                entry=entry->next) {
                    count++;
            }
            return count;
        }
    }


    int RegisterDescription::AssignDestinationRegister(List<Instruction*> *entry,Instruction *defInstruction,MachineRegisters *machineRegisters,RegisterDescription **registerFile,
        Instruction **prefix) {
            if (index==0)
                return 0;
            /*
            if (IsArgOutDef(defInstruction)) {
                location=machineRegisters->GetScratchReg()->index;
                return location;
            }
            */
            lastAccessEntry=entry;
            switch (regType) {
                case RegisterTypeIntConstant:       
                    // holds an integer constant
                case RegisterTypeDoubleConstant:   
                    // holds a double constant
                case RegisterTypeObjectConstant:    
                    // holds a constant object (such as undefined or global)
                    return index; // for now, assume we can synthesize this constant locally
                case RegisterTypeBlockScope:  
                    // variable or temporary used only within a single block
                    if (location==NO_ASSIGNMENT) {
#define SMOOT
#ifdef SMOOT
                        if (IsTemporaryDef(entry,defInstruction,registerFile)) {
                            location=machineRegisters->GetTempReg()->index;
                        }
                        else {
                            MachineRegister *machineRegister=GetTempRegister(defInstruction,machineRegisters,registerFile,prefix,NULL);
                            location=machineRegister->index;
                            machineRegister->currentAssignment=index;
                        }
#else
                        MachineRegister *machineRegister=GetTempRegister(defInstruction,machineRegisters,registerFile,prefix,NULL);
                        location=machineRegister->index;
                        machineRegister->currentAssignment=index;
#endif
                    }
                    return location;
                case RegisterTypeFunctionScope:      
                    // variable or argument used across blocks
                    if (location==NO_ASSIGNMENT) {
                        MachineRegister *machineRegister=GetVarRegister(machineRegisters,registerFile,prefix,NULL);
                        location=machineRegister->index;
                        machineRegister->currentAssignment=index;
                    }
                    return location;
                case RegisterTypeUnknown: 
                default:
                    AssertMsg(false,"Unknown register type in AssignDestinationRegister");
                    return 0;
            }
    }

    void RegisterDescription::SetArgIn(int argIndex,Instruction *defInstruction) {
        regType=RegisterTypeFunctionScope;
        AddDef(defInstruction);
        this->argIndex=argIndex;
        // FIX: TODO: generalize for non-CDECL 
        // offset is 16 because first two arguments are function wrapper and number of args
        framePointerOffset=16+(argIndex*4);
    }

    int RegisterDescription::GetNumberOfAccesses(BasicBlock *block) {
        if (nAccesses<0) {
            nAccesses=0;
            bool prevIsUse=true; // if first access is a use, then it counts 
            for (List<Instruction*> *entry=accesses->next;!(entry->isHead);entry=entry->next) {
                if (entry->data->basicBlock==block) {
                    if (IsDef(entry->data)) {
                        prevIsUse=false;
                    }
                    else {
                        if (prevIsUse) {
                            nAccesses++;
                        }
                        prevIsUse=true;
                    }
                }
            }
        }
        return nAccesses;
    }

    void RegisterDescription::Characterize() {
        if (regType==RegisterTypeUnknown) {
            regType=RegisterTypeBlockScope;
            localBlock=NULL;
            for (List<Instruction*> *entry=accesses->next;!(entry->isHead);entry=entry->next) {
                if (localBlock==NULL)
                    localBlock=entry->data->basicBlock;
                else if (localBlock!=entry->data->basicBlock) {
                    regType=RegisterTypeFunctionScope;
                    break;
                }
            }
        }
    }

