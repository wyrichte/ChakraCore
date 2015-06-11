/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
int __cdecl compareOpCounts(const void *a,const void *b);

struct BlockStats {
    BasicBlock *block;
    int execCount;
};

class InstructionStats {
    List<BlockStats*> *blocks;
    ArenaAllocator *alloc;
    OpCount *opHistogram;
    unsigned int nOps;
public:
    InstructionStats(ArenaAllocator *alloc) : alloc(alloc) {
        blocks=ListFn<BlockStats*>::MakeListHead(alloc);
        nOps=(unsigned int)Js::OpCode::Last;
        opHistogram=(OpCount *)alloc->Alloc(nOps*sizeof(OpCount));
        for (unsigned int i=0;i<nOps;i++) {
            opHistogram[i].op=i;
            opHistogram[i].count=0;
        }
    }

    void AddTracedBlock(BlockStats *blockStats) {
        ListFn<BlockStats*>::Add(blocks,blockStats,alloc);
    }

    void PrintInstructionTrace();
};

