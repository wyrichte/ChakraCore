//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    CriticalSection deleteListLock;

    struct ReleaseNode 
    {
        IUnknown *unk;              // Assume non-null
        ThreadContextId threadId;   // Assume non-null
        ReleaseNode *next;          // Assume a grounded type that will terminate in a null.

        ReleaseNode(IUnknown *unk, ThreadContextId threadId): unk(unk), threadId(threadId), next(nullptr) { }
    };

    ReleaseNode *unkList = nullptr;

    void PrependRelease(ReleaseNode *node)
    {
        AutoCriticalSection lock(&deleteListLock);
        
        // unkList is a grounded type. Adding to the beginning of a grounded type still preserves a grounded type.
        node->next = unkList;
        unkList = node;
    }

    void ScheduleRelease(IUnknown *unk, ThreadContextId threadId)
    {
        // Ensure we match the data-type assumptions.
        Assert(unk);
        Assert(threadId);

        PrependRelease(new ReleaseNode(unk, threadId));
    }

    void PerformScheduledReleases()
    {
        ReleaseNode *list;

        // Retrieve the list while preventing a concurrent call to ScheduleRelease from adding to it while we are fetching the list.
        {
            AutoCriticalSection lock(&deleteListLock);

            // It looks like we could use InterlockedExchange() here but we can't as we need to prevent PrependRelease from being
            // in the middle of execution because there is no way to atomically add to the beginning of a list (requires a read and 
            // two non-contiguous writes being atomic). Even though we can do the atomic exchange here, PrependRelease's read might 
            // read the old value and miss the nullptr write causing (at best) a later crash, put possibly an infinite loop.
            list = unkList;

            // Null is a trivially grounded type.
            unkList = nullptr;
        }

        // We can safely ignore concurrent adds as we assume we will be called periodically meaning that current adds will just be
        // handled in the next call to PerformScheduledReleases().

        // Only perform releases for this thread.
        auto currentThread = GetCurrentThreadContextId();

        // Perform the scheduled releases while deleting the list itself.
        // This loop will terminate because list is grounded (see grounded comments above).
        auto current = list;
        while (current)
        {
            // Fetch next now because, either the node will be delete, potentially making access to next throw, or next will be overwritten
            // when the node is added back to the list.
            auto next = current->next;

            if (current->threadId == currentThread) 
            {    
                // We are on the right thread, release the object.
                current->unk->Release();
                delete current;
            }
            else
            {
                // This is not the right thread to release this object, put it back on the list for the other thread.
                PrependRelease(current);
            }

            current = next;
        }
    }

}
