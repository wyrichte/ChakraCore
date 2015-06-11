// Copyright (C) Microsoft. All rights reserved. 
#include "stdafx.h"

#ifdef PROFILE_EXEC
namespace Js 
{
    ULONG
    ScriptContextProfiler::AddRef()
    {
        return refcount++;
    }
    
    ULONG
    ScriptContextProfiler::Release()
    {
        ULONG count = --refcount;
        if (count == 0)
        {            
            if (recycler != null && this->profiler == recycler->GetProfiler())
            {
                recycler->SetProfiler(null, null);
            }
            NoCheckHeapDelete(this);
        }
        return count;
    }

    ScriptContextProfiler::ScriptContextProfiler() :
        refcount(1), profilerArena(null), profiler(null), backgroundRecyclerProfilerArena(null), backgroundRecyclerProfiler(null), recycler(null), pageAllocator(null), next(null)
    {                
    }
    
    void
    ScriptContextProfiler::Initialize(PageAllocator * pageAllocator, Recycler * recycler)
    {        
        Assert(!IsInitialized());
        profilerArena = HeapNew(ArenaAllocator, L"Profiler", pageAllocator, Js::Throw::OutOfMemory);
        profiler = Anew(profilerArena, Profiler, profilerArena);
        if (recycler)
        {
            backgroundRecyclerProfilerArena = recycler->AddBackgroundProfilerArena();
            backgroundRecyclerProfiler = Anew(profilerArena, Profiler, backgroundRecyclerProfilerArena);

#if DBG
            //backgroundRecyclerProfiler is allocated from background and its gauranteed to assert below if we don't disable thread access check.
            backgroundRecyclerProfiler->alloc->GetPageAllocator()->SetDisableThreadAccessCheck();
#endif

            backgroundRecyclerProfiler->Begin(Js::AllPhase);

#if DBG
            backgroundRecyclerProfiler->alloc->GetPageAllocator()->SetEnableThreadAccessCheck();
#endif
        }
        profiler->Begin(Js::AllPhase);

        this->recycler = recycler;
    }

    void
    ScriptContextProfiler::ProfilePrint(Js::Phase phase)
    {
        if (!IsInitialized())
        {
            return;
        }        
        profiler->End(Js::AllPhase);
        profiler->Print(phase);
        if (this->backgroundRecyclerProfiler)
        {
            this->backgroundRecyclerProfiler->End(Js::AllPhase);
            this->backgroundRecyclerProfiler->Print(phase);
            this->backgroundRecyclerProfiler->Begin(Js::AllPhase);
        }
        profiler->Begin(Js::AllPhase);
    }

    ScriptContextProfiler::~ScriptContextProfiler()
    {       
        if (profilerArena)
        {
            HeapDelete(profilerArena);
        }

        if (recycler && backgroundRecyclerProfilerArena)
        {
#if DBG
            //We are freeing from main thread, disable thread check assert. 
            backgroundRecyclerProfilerArena->GetPageAllocator()->SetDisableThreadAccessCheck();
#endif
            recycler->ReleaseBackgroundProfilerArena(backgroundRecyclerProfilerArena);
        }
    }

    void
    ScriptContextProfiler::ProfileBegin(Js::Phase phase)
    {
        Assert(IsInitialized());
        this->profiler->Begin(phase);
    }

    void
    ScriptContextProfiler::ProfileEnd(Js::Phase phase)
    {
        Assert(IsInitialized());
        this->profiler->End(phase);
    }

    void
    ScriptContextProfiler::ProfileSuspend(Js::Phase phase, Js::Profiler::SuspendRecord * suspendRecord)
    {
        Assert(IsInitialized());
        this->profiler->Suspend(phase, suspendRecord);
    }

    void
    ScriptContextProfiler::ProfileResume(Js::Profiler::SuspendRecord * suspendRecord)
    {
        Assert(IsInitialized());
        this->profiler->Resume(suspendRecord);
    }

    void
    ScriptContextProfiler::ProfileMerge(ScriptContextProfiler * profiler)
    {
        Assert(IsInitialized());
        this->profiler->Merge(profiler->profiler);
    }
}
#endif
