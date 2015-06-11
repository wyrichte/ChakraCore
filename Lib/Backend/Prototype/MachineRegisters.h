/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#define NO_ASSIGNMENT (-1)

struct MachineRegister {
    int index;
    bool isCalleeSaved;
    bool isFramePointer;
    bool isStackPointer;
    bool isTemp;
    bool isScratch;
    int currentAssignment;  // NO_ASSIGNMENT if no byte code register is in this register
    MachineRegister *next;  // next pointer on free list
};

// TODO: floating-point registers 

class MachineRegisters {
    MachineRegister **intRegisters;
    int freeRegs;
    int tempRegister;
    int scratchRegister;
    int framePointer;
    int stackPointer;
    int nIntRegs;
    ArenaAllocator *alloc;
    int robin;
    int nLocations;
public:
    MachineRegisters(ArenaAllocator *alloc,int nIntRegs,int tempRegister,int scratchRegister,int framePointer,int stackPointer,unsigned int calleeSaved)
        : freeRegs(0),tempRegister(tempRegister), scratchRegister(scratchRegister), framePointer(framePointer), stackPointer(stackPointer), 
        nIntRegs(nIntRegs),alloc(alloc), robin(1) {
            intRegisters=(MachineRegister**)alloc->Alloc(nIntRegs*sizeof(MachineRegister*));
            for (int i=0;i<nIntRegs;i++) {
                int calleeSavedMask=1<<i;  // mask into calleeSaved information
                MachineRegister *machineRegister=MakeStruct(alloc,MachineRegister);
                intRegisters[i]=machineRegister;
                machineRegister->next = 0;
                machineRegister->currentAssignment=NO_ASSIGNMENT;
                bool isCalleeSaved=(0!=(calleeSavedMask&calleeSaved));
                machineRegister->isCalleeSaved=isCalleeSaved;
                machineRegister->index=i;
                machineRegister->isTemp=(i==tempRegister);
                machineRegister->isScratch=(i==scratchRegister);
                machineRegister->isFramePointer=(i==framePointer);
                machineRegister->isStackPointer=(i==stackPointer);
                if (!(machineRegister->isScratch||machineRegister->isTemp||machineRegister->isFramePointer||machineRegister->isStackPointer)) {
                    FreeReg(i);
                }
            }
    }

    static bool IsUsed(Instruction *useInstruction,int sourceReg);
    MachineRegister *FindSpillCandidate(Instruction *useInstruction,RegisterDescription **registerDescriptions);
    void HomeAndFreeAll(bool insertBefore,BasicBlock *block,List<Instruction*> *instructionsInBlock,RegisterDescription **registerFile);

    void FreeReg(int i) {
        MachineRegister *reg=intRegisters[i];
        reg->currentAssignment=NO_ASSIGNMENT;
        freeRegs|=(1<<i);
    }

    MachineRegister *CompeteForRegister(int nAccesses,BasicBlock *block,RegisterDescription **registerDescriptions);
    MachineRegister *GetTempReg() {
        return intRegisters[tempRegister];
    }

    MachineRegister *GetScratchReg() {
        return intRegisters[scratchRegister];
    }

    MachineRegister *GetFreeVarReg();
};

