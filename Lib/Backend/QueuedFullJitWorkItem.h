//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

struct InMemoryCodeGenWorkItem;

class QueuedFullJitWorkItem : public JsUtil::DoublyLinkedListElement<QueuedFullJitWorkItem>
{
private:
    InMemoryCodeGenWorkItem *const workItem;

public:
    QueuedFullJitWorkItem(InMemoryCodeGenWorkItem *const workItem);

public:
    InMemoryCodeGenWorkItem *WorkItem() const;
};
