//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

// For prefetch
#include <mmintrin.h>


MarkContext::MarkContext(Recycler * recycler, PagePool * pagePool) : 
    recycler(recycler),
    pagePool(pagePool),
    markStack(pagePool),
    trackStack(pagePool)
{
}


MarkContext::~MarkContext() 
{
#ifdef RECYCLER_MARK_TRACK
    this->markMap = null;
#endif
}

#ifdef RECYCLER_MARK_TRACK
void MarkContext::OnObjectMarked(void* object, void* parent)
{
    if (!this->markMap->ContainsKey(object))
    {
        this->markMap->AddNew(object, parent);
    }
}
#endif

void MarkContext::Init()
{
    markStack.Init();
    trackStack.Init();
}

void MarkContext::Clear()
{
    markStack.Clear();
    trackStack.Clear();
}

void MarkContext::Abort()
{
    markStack.Abort();
    trackStack.Abort();

    pagePool->ReleaseFreePages();
}


void MarkContext::Release()
{
    markStack.Release();
    trackStack.Release();

    pagePool->ReleaseFreePages();
}


uint MarkContext::Split(uint targetCount, __in_ecount(targetCount) MarkContext ** targetContexts)
{
    Assert(targetCount > 0 && targetCount <= PageStack<MarkCandidate>::MaxSplitTargets);
    __analysis_assume(targetCount <= PageStack<MarkCandidate>::MaxSplitTargets);
    
    PageStack<MarkCandidate> * targetStacks[PageStack<MarkCandidate>::MaxSplitTargets];

    for (uint i = 0; i < targetCount; i++)
    {
        targetStacks[i] = &targetContexts[i]->markStack;
    }
    
    return this->markStack.Split(targetCount, targetStacks);
}


void MarkContext::ProcessTracked()
{
    if (trackStack.IsEmpty())
    {
        return;
    }

    FinalizableObject * trackedObject;
    while (trackStack.Pop(&trackedObject))
    {
        MarkTrackedObject(trackedObject);

        // TODO: Stats etc
//            RECYCLER_STATS_INC(this, trackedObjectCount);
    }

    Assert(trackStack.IsEmpty());

    trackStack.Release();
}



