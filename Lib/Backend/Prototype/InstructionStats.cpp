/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

void InstructionStats::PrintInstructionTrace() {
    for (List<BlockStats*> *entry=blocks->next;!(entry->isHead); entry=entry->next) {
        printf("block at start offset %x ran %d times\n",entry->data->block->GetStartOffset(),
            entry->data->execCount);
        entry->data->block->AddOpCodeCounts(opHistogram,entry->data->execCount);
    }
    printf("Instruction Counts:\n");
    qsort(opHistogram,nOps,sizeof(OpCount),compareOpCounts);
    for (unsigned int i=0;i<nOps;i++) {
        if (opHistogram[i].count>0)
            printf("  Op %-10s: %d\n",Js::OpCodeNames[opHistogram[i].op],opHistogram[i].count);
    }
}


