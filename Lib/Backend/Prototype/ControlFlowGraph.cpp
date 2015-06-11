/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

void ControlFlowGraph::InsertBlockInOrder(BasicBlock *block) {
    for (List<BasicBlock*> *entry=blocks->next;!(entry->isHead);entry=entry->next) {
        if (entry->data->GetStartOffset()>block->GetStartOffset()) {
            List<BasicBlock*> *insertedEntry=ListFn<BasicBlock*>::MakeListEntry(alloc);
            insertedEntry->data=block;
            ListFn<BasicBlock*>::InsertBefore(entry,insertedEntry);
            return;
        }
    }
    AddBlock(block);
}

void ControlFlowGraph::Print(MemoryContext* memContext) {
    for (List<BasicBlock*> *entry=blocks->next;!(entry->isHead);entry=entry->next) {
        entry->data->Print(memContext);
    }
}

