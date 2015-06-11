/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class ControlFlowGraph {
    BasicBlock *entry;
    BasicBlock *exit;
    List<BasicBlock*> *blocks;
    ArenaAllocator *alloc;
public:
    ControlFlowGraph(ArenaAllocator *alloc) : alloc(alloc) {
        entry=Anew(alloc,BasicBlock,alloc,0);
        blocks=ListFn<BasicBlock*>::MakeListHead(alloc);
        AddBlock(entry);
        // exit is not put on the list of blocks
        exit=Anew(alloc,BasicBlock,alloc,-1);
    }

    void AddBlock(BasicBlock *block) {
        ListFn<BasicBlock*>::Add(blocks,block,alloc);
    }

    void InsertBlockInOrder(BasicBlock *block);

    BasicBlock *GetEntry() {
        return entry;
    }

    BasicBlock *GetExit() {
        return exit;
    }

    List<BasicBlock*> *GetBlocks() {
        return blocks;
    }
    void Print(MemoryContext* memContext);
};
