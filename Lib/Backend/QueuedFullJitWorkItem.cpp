//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "BackEnd.h"

QueuedFullJitWorkItem::QueuedFullJitWorkItem(InMemoryCodeGenWorkItem *const workItem) : workItem(workItem)
{
    Assert(workItem->GetJitMode() == ExecutionMode::FullJit);
}

InMemoryCodeGenWorkItem *QueuedFullJitWorkItem::WorkItem() const
{
    return workItem;
}
