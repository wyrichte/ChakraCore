//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "static\MemProtectHeapPch.h"
#include <setjmp.h>
#include "MemProtectHeap.h"

// Initialization order
//  AB AutoSystemInfo
//  AD PerfCounter
//  AE PerfCounterSet
//  AM Output/Configuration
//  AN MemProtectHeap
//  AP DbgHelpSymbolManager
//  AQ CFGLogger
//  AR LeakReport
//  AS JavascriptDispatch/RecyclerObjectDumper
//  AT HeapAllocator/RecyclerHeuristic
//  AU RecyclerWriteBarrierManager
#pragma warning(disable:4075)
#pragma init_seg(".CRT$XCAN")

//////////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////////

#define FailFast(msg) AssertMsg(false, msg); RaiseFailFastException(NULL, NULL, 0);

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn569307(v=vs.85).aspx for details about this value
#define MEMPROTECT_HEAP_SOURCE 3

//////////////////////////////////////////////////////////////////////////////
// ThreadContext
//
//   This class holds the per protected thread registration and handles
//   the work of getting threads to safe points for collection.
//////////////////////////////////////////////////////////////////////////////

class MemProtectHeap;

class MemProtectThreadContext : public JsUtil::DoublyLinkedListElement<MemProtectThreadContext>
{
    // We report local unroot statistics to the heap this many bytes of unroot calls of a given type
    static const size_t LocalUnrootReportFrequency = 128 * 1024;

public:
    enum class ThreadState : LONG
    {
        // This is the default state for a protected thread. It indicates that the
        // thread-local-allocations are enable (if available) and requires the GC to
        // scan this thread's stack durring GC.
        Protected = 0,

        // This flag is set by the GC, while holding the GC lock, indicating that a safe point
        // is requested, and that the GC may be waiting to be signaled after the thread enters
        // the Stopped state.
        StopRequest = 1,

        // This flag indicates that the thread has reached a safe point, and is waiting to
        // acquire the GC lock.
        Stopped = 2,
    };

    // This class marks a region of code where the thread can not be safely suspended
    class NoSuspendRegion
    {
    public:
        NoSuspendRegion(MemProtectThreadContext* threadContext) : threadContext(threadContext)
        {
            Assert(!this->threadContext->inNoSuspendRegion);
            this->threadContext->inNoSuspendRegion = true;
        }
        ~NoSuspendRegion()
        {
            this->threadContext->inNoSuspendRegion = false;
        }
    private:
        MemProtectThreadContext* threadContext;
    };


    MemProtectThreadContext(MemProtectHeap* memProtectHeap, MemProtectCooperativeThreadWakeProc wakeProc, void* wakeArgument);

    ~MemProtectThreadContext();

    // Make an (asynchronous) asuspend request
    void RequestSuspension();

    // Ensure the thread is actually suspended (and in a safe location).
    void EnsureSuspended();

    // Allow a thread to resume from suspension
    void ResumeFromSuspension();

    // Attempt to switch this thread from protected to having a stop request.
    void SetStopRequest()
    {
        ThreadState old = (ThreadState)InterlockedCompareExchange(&(LONG&)state, (LONG)ThreadState::StopRequest, (LONG)ThreadState::Protected);
        Assert(old != ThreadState::StopRequest);
    }

    //check if the thread exits because that TerminateThread is invoked from the thread itself
    //If yes, we will not be notified to set disableStackScanningDueToLeakedThread.
    //We need to set it here once we have confirmed the thread exits already.
    bool IsLeakedThread()
    {
        if ( this->disableStackScanningDueToLeakedThread )
            return true;

        if( WaitForSingleObject(this->hThread, 0) != WAIT_TIMEOUT)
        {
            this->disableStackScanningDueToLeakedThread = true;
        }
        return this->disableStackScanningDueToLeakedThread;
    }

    // Check if the thread is running (not stopped)
    bool IsRunning()
    {
        return state != ThreadState::Stopped && !this->IsLeakedThread();
    }

    // Invoke the cooperative callback (if one exists for this context)
    void CallWakeProc();

    // Acquire the heap critical section, ensuring that this context is in a safe state to ensure collector progress
    CriticalSection* SafepointEnterCriticalSection();

    // Post-unroot notification (allows us to accumulate unroots in the thread local structure)
    void NotifyUnroot(RecyclerHeapObjectInfo const& heapObjectInfo);

    // Collection entry point from this thread context. Accumulates local statistics to the heap and delegates.
    BOOL Collect(MemProtectHeapCollectFlags flags);

    // Spill all registers for the current thread into the given buffer and update the stack bounds
    void SaveContextAndUpdateBounds(jmp_buf& context);

    // Scan the stack corresponding to this thread context
    size_t ScanStack(RecyclerScanMemoryCallback& scanMemory);

    // Simple accessor for recycler (needs implementation after MemProtectHeap definition)
    Recycler* GetRecycler();

    // Simple accessor for thread id
    DWORD GetThreadId() { return threadId; }

    // Emergency disable protection for the current thread. This should only occur if the thread leaks its UnprotectCurrentThread()
    // status and we've reached DllMain. UnprotectCurrentThread() is not DllMain safe, so we disable the thread instead.
    void NotifyThreadDetach()
    {
        disableStackScanningDueToLeakedThread = true;
    }

private:
    // A flag that indicates if the thread is in a region where suspend is not safe
    volatile bool inNoSuspendRegion;

    // Thread reached DllMain without unprotecting, and has been leaked. Disable stack scans, as thread may have exited.
    bool disableStackScanningDueToLeakedThread;

    // The current thread state (used in cooperative mode)
    volatile ThreadState state;

    // A reference to the heap this thread is registered with
    MemProtectHeap* memProtectHeap;

    // Local statistics for unroot calls.
    size_t localUnrootSize;
    size_t localUnmarkedUnrootSize;

    // The bounds of the stack for the current safe/suspend point
    void* stackTop;
    void* stackBase;

    // Cooperative suspension callback for this thread
    MemProtectCooperativeThreadWakeProc wakeProc;
    void* wakeArgument;

    // The Windows thread id and handle
    DWORD threadId;

    // The Windows thread handle
    HANDLE hThread;

    // A pointer to the thread information block.
    NT_TIB* tib;

    // Place to store context for GetThreadContext
    CONTEXT context;
};

//////////////////////////////////////////////////////////////////////////////
// SafepointAutoCriticalSection
//
//   This class does the work of ensuring that when we block entering a
//   critical section that we allow collection to proceed.
//////////////////////////////////////////////////////////////////////////////

class SafepointAutoCriticalSection
{
public:
    SafepointAutoCriticalSection(MemProtectThreadContext* threadContext)
    {
        this->criticalSection = threadContext->SafepointEnterCriticalSection();
    }

    ~SafepointAutoCriticalSection()
    {
        this->criticalSection->Leave();
    }

private:
    CriticalSection* criticalSection;
};

//////////////////////////////////////////////////////////////////////////////
// AutoOutsideOfCriticalSection
//
//   Releases a critical section for a defined period of time.
//////////////////////////////////////////////////////////////////////////////

class AutoOutsideOfCriticalSection
{
public:
    AutoOutsideOfCriticalSection(CriticalSection* cs) : cs(cs)
    {
        cs->Leave();
        Assert(!cs->IsLocked());
    }

    ~AutoOutsideOfCriticalSection()
    {
        cs->Enter();
    }

private:
    CriticalSection* cs;
};

//////////////////////////////////////////////////////////////////////////////
// MemProtectRecyclerCollectionWrapper
//
//   Manages all the callbacks from the Recycler to allow safe points to be
//   implemented correctly.
//////////////////////////////////////////////////////////////////////////////

class MemProtectRecyclerCollectionWrapper : public RecyclerCollectionWrapper
{
public:
    MemProtectRecyclerCollectionWrapper(MemProtectHeap* memProtectHeap) :
        memProtectHeap(memProtectHeap),
        inSafepointRegion(false) {}

    virtual void PreCollectionCallBack(CollectionFlags flags) override;
    virtual void PreSweepCallback() override {}
    virtual void PreRescanMarkCallback() override;
    virtual void RescanMarkTimeoutCallback() override;
    virtual void EndMarkCallback() override;
    virtual size_t RootMarkCallback(RecyclerScanMemoryCallback& scanMemoryCallback, BOOL* stacksScannedByRuntime) override;
    virtual void ConcurrentCallback() override;
    virtual void WaitCollectionCallBack() override {}
    virtual void PostCollectionCallBack() override;
    virtual BOOL ExecuteRecyclerCollectionFunction(Recycler* recycler, CollectionFunction function, CollectionFlags flags) override;
    virtual uint GetRandomNumber() override { return 0; }
    virtual bool DoSpecialMarkOnScanStack() override { return false; }
    virtual void PostSweepRedeferralCallBack() override {}
#ifdef FAULT_INJECTION
    virtual void DisposeScriptContextByFaultInjectionCallBack() override {};
#endif
    virtual void DisposeObjects(Recycler* recycler) override;
    virtual void PreDisposeObjectsCallBack() override {};
    
#ifdef ENABLE_PROJECTION
    virtual void MarkExternalWeakReferencedObjects(bool inPartialCollect) override {};
    virtual void ResolveExternalWeakReferencedObjects() override {};
#endif
#if DBG || defined(PROFILE_EXEC)
    virtual bool AsyncHostOperationStart(void*) override { return false; };
    virtual void AsyncHostOperationEnd(bool wasInAsync, void*) override {};
#endif

private:
    // The heap we are associated with
    MemProtectHeap* memProtectHeap;

    // keep track of if we have started a safepoint (there are one or two callbacks that require it)
    bool inSafepointRegion;
};

//////////////////////////////////////////////////////////////////////////////
// MemProtectHeap
//
//   The main class that manages the shared state of a heap
//////////////////////////////////////////////////////////////////////////////

class MemProtectHeap
{
    // When an explicit call to run heuristics is made (i.e., an idle callback) use this limit
    static const size_t ExplicitUnrootLimit = 128 * 1024;

    // The base cost when computing an unroot limit
    static const size_t UnrootLimitBase = 512 * 1024;

    // The heap-size related cost when computing an unroot limit
    static const size_t UnrootLimitHeapFraction = 6;

    friend class MemProtectRecyclerCollectionWrapper;

    // The mode used for protecting stacks
    enum StackProtectionMode
    {
        CurrentThreadOnly,
        MultipleThreadsSuspend,
        MultipleThreadsCooperative
    };

public:
    MemProtectHeap(MemProtectHeapCreateFlags flags) :
        threadContextTlsIndex(TLS_OUT_OF_INDEXES),
        stackProtectionMode(StackProtectionMode::CurrentThreadOnly),
        safepointEvent(NULL),
        disallowCollection(false),
        processExiting(false),
        forceInThread(false),
        headRootSection(nullptr),
        criticalSection(4000),
        recyclerCollectionWrapper(this),
        pageAllocator(nullptr, PageAllocatorType_Max, ConfigurationLoader::Flags(), 0, PageAllocator::DefaultMaxFreePageCount, false, &backgroundPageQueue), /* TODO: Perf counter? */
        recycler(nullptr, &pageAllocator, &MemProtectHeap::OutOfMemory, ConfigurationLoader::Flags()),
        unrootSize(0),
        unmarkedUnrootSize(0),
        unrootLimit(UnrootLimitBase),
        partialCollection(false)
    {
        // Check for overrides for thread suspension or cooperative safepoints
        if (flags & MemProtectHeapCreateFlags_ProtectMultipleStacksSuspend)
        {
            if (flags & MemProtectHeapCreateFlags_ProtectMultipleStacksCooperative)
            {
                FailFast("ProtectMultipleStacksSuspend and ProtectMultipleStacksCooperative are mutually exclusive.");
            }

            if (flags & MemProtectHeapCreateFlags_ForceInThreadCollection)
            {
                FailFast("ProtectMultipleStacksSuspend is not compatible with ForceInThreadCollection.");
            }

            this->stackProtectionMode = StackProtectionMode::MultipleThreadsSuspend;
        }

        if (flags & MemProtectHeapCreateFlags_ProtectMultipleStacksCooperative)
        {
            this->stackProtectionMode = StackProtectionMode::MultipleThreadsCooperative;
        }

#ifdef RECYCLER_PAGE_HEAP
        PageHeapMode pageHeapMode = PageHeapMode::PageHeapModeOff;
        bool pageHeapModeCaptureAllocStack = false;
        bool pageHeapModeCaptureFreeStack = false;
#endif

        if ((flags & MemProtectHeapCreateFlags_PageHeapModeBlockStart)
            && (flags & MemProtectHeapCreateFlags_PageHeapModeBlockEnd))
        {
            FailFast("Pageheap: Guard page on both block start and block end is not supported");
        }
        else
        {
            if ((flags & MemProtectHeapCreateFlags_PageHeapModeBlockStart))
            {
                pageHeapMode = PageHeapMode::PageHeapModeBlockStart;
            }
            else if ((flags & MemProtectHeapCreateFlags_PageHeapModeBlockEnd))
            {
                pageHeapMode = PageHeapMode::PageHeapModeBlockEnd;
            }
        }

#ifdef RECYCLER_PAGE_HEAP
        if (pageHeapMode != PageHeapMode::PageHeapModeOff)
        {
            if ((flags & MemProtectHeapCreateFlags_PageHeapModeCaptureAllocStack) != 0)
            {
                pageHeapModeCaptureAllocStack = true;
            }
            if ((flags & MemProtectHeapCreateFlags_PageHeapModeCaptureFreeStack) != 0)
            {
                pageHeapModeCaptureFreeStack = true;
            }
        }
#endif

        // Did the creator request always in-thread (synchronous) collections?
        this->forceInThread = (flags & MemProtectHeapCreateFlags_ForceInThreadCollection) != 0;

        // Allocate the thread local slot
        this->threadContextTlsIndex = TlsAlloc();
        if (this->threadContextTlsIndex == TLS_OUT_OF_INDEXES)
        {
            FailFast("Failed to allocate Tls index");
        }

        if (this->InCooperativeMultipleThreadMode())
        {
            // Allocate the event for cooperative safepoints
            this->safepointEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (this->safepointEvent == NULL)
            {
                FailFast("Failed to create safepoint event");
            }
        }

        // Initialize the recycler with deferred thread startup
        bool deferThreadStartup = this->InMultipleThreadMode() && !this->forceInThread;
        this->recycler.Initialize(this->forceInThread, nullptr, deferThreadStartup
#ifdef RECYCLER_PAGE_HEAP
            , pageHeapMode
            , pageHeapModeCaptureAllocStack
            , pageHeapModeCaptureFreeStack
#endif
            );
        this->recycler.SetMemProtectMode();
        this->recycler.SetCollectionWrapper(&recyclerCollectionWrapper);

#if DBG
        // Disable thread access checks (we support multiple threads)
        this->pageAllocator.SetDisableThreadAccessCheck();
        this->recycler.SetDisableThreadAccessCheck();
#endif
    };

    ~MemProtectHeap()
    {
        // Enter the critical section here to make invariants hold in collections from ~Recycler()
        this->criticalSection.Enter();

        if (!this->InMultipleThreadMode() && !this->forceInThread)
        {
            // We stop the concurrent collector thread on destruction if we are not in multi-threaded mode
            this->recycler.ShutdownThread();
        }

        this->pageAllocator.Close();
        TlsFree(this->threadContextTlsIndex);

        if (this->InCooperativeMultipleThreadMode())
        {
            CloseHandle(this->safepointEvent);
        }
    }

    // Called when a thread reaches a safepoint to notify the coordinating thread.
    void SignalSafepoint();

    // Post-unroot notification (allows us to accumulate unroot statistics)
    void NotifyUnroot(RecyclerHeapObjectInfo const& heapObjectInfo);

    // A collection has started
    void NotifyCollectionStart(bool partialCollection);

    // A collection has finished
    void NotifyCollectionEnd();

    // Log ETW event with heap size information
    void LogHeapSize(bool fromGC);

    // Collect after transferring some unroot statistics. Returns true if a collection occured.
    BOOL Collect(MemProtectHeapCollectFlags flags, size_t localUnrootSize, size_t localUnmarkedUnrootSize);

    // Main collection entrypoint (including running the heuristics). Returns true if a collection occured.
    BOOL Collect(MemProtectHeapCollectFlags flags);

    // Attach/detach thread contexts from this heap
    void AttachThreadContext(MemProtectThreadContext* threadContext);
    void DetachThreadContext(MemProtectThreadContext* threadContext);

    // Add/Remove protection for the current thread (create/free the context if necessary)
    void ProtectCurrentThread(MemProtectCooperativeThreadWakeProc wakeProc, void* wakeArgument);
    HRESULT UnprotectCurrentThread();

    // Manage the registration of explicit (non-stack) root sections.
    void AddRootSection(MemProtectHeapRootSection* rootSection);
    bool RemoveRootSection(MemProtectHeapRootSection* rootSection);

    // During process exit we may not safely be able to collect (exit is not graceful), so we avoid collection.
    void DisableCollectionForProcessExit() { this->processExiting = true; }

    // Return the thread context for the currently executing thread (or null if this thread is not attached)
    MemProtectThreadContext* CurrentThreadContext()
    {
        Assert(this->threadContextTlsIndex != TLS_OUT_OF_INDEXES);
        return reinterpret_cast<MemProtectThreadContext*>(TlsGetValue(this->threadContextTlsIndex));
    }

    // Simple accessors
    bool InSuspendMultipleThreadMode() { return (this->stackProtectionMode == StackProtectionMode::MultipleThreadsSuspend); }
    bool InCooperativeMultipleThreadMode() { return (this->stackProtectionMode == StackProtectionMode::MultipleThreadsCooperative); }
    bool InMultipleThreadMode() { return (this->stackProtectionMode != StackProtectionMode::CurrentThreadOnly); }
    Recycler* GetRecycler() { return &this->recycler; }
    CriticalSection* GetCriticalSection() { return &this->criticalSection; }
    void Destroy();

#if DBG
    // Disable exit checks (we may exit too late to allow synchronizing with threads due to the loader lock).
    void SetDisableConcurentThreadExitedCheck()
    {
        recycler.SetDisableConcurrentThreadExitedCheck();
    }

    // Parse the given command line options into the config table.
    static void ParseCommandLine(int argc, __in_ecount(argc) LPWSTR argv[])
    {
        CmdLineArgsParser parser(nullptr, ConfigurationLoader::Flags());
        parser.Parse(argc, argv);
    }
#endif

private:
    // Callback the recycler uses if we hit out of memory.
    static void  __declspec(noreturn) OutOfMemory();

    // Is it safe to collect? (there are some time windows where we can not collect)
    bool CanCollect()
    {
        Assert(this->GetCriticalSection()->IsLocked());
        return !this->disallowCollection && !this->processExiting;
    }

    // Stop all protected threads (either using suspend or cooperatively)
    void StopAllThreads(bool inThread);

    // Release all threads from the safepoint
    void ResumeAllThreads();

    // Makes it safe to call suspend (e.g., we do not suspend threads while holding important locks)
    void BeginSuspendProtection();

    // Removes suspend protection (by this point we must guarantee all threads are actually suspended)
    void EndSuspendProtection();

    // Scan all of the roots known by this heap (including stacks and root sections)
    size_t FindRoots(RecyclerScanMemoryCallback& scanMemory);

    // Enumerate and scan all protected stacks
    size_t ScanProtectedStacks(RecyclerScanMemoryCallback& callBack);

    // Leave the critical section and then start and synchronize with the collector threads.
    void StartCollectorThreadsOutsideOfCriticalSection();

    // Finish any collection, request all collector stop, and then synchronize with them outside the critical section.
    void StopCollectorThreadsOutsideOfCriticalSection();

private:
    // This class parses configuration sources on construction
    class ConfigurationLoader
    {
    private:
        // We have to initialize this earlier to get defaults correctly.
        static Js::Configuration s_userConfig;

        static ConfigurationLoader configurationLoader;

        // This constructor loads the configuration when the singleton is first used.
        ConfigurationLoader();

    public:
        // This method holds the static singleton
        static Js::ConfigFlagsTable& Flags();
    };

    // The index for the tls slot for this heap
    DWORD threadContextTlsIndex;

    // Current limit for triggering the next collection
    size_t unrootLimit;

    // Overall count of unrooted bytes.
    size_t unrootSize;
    size_t unmarkedUnrootSize;

    // Number of unrooted bytes at the start of the last collection.
    size_t oldUnrootSize;
    size_t oldUnmarkedUnrootSize;

    // Are we currently performing a partial collection
    bool partialCollection;

    // True if this heap is using suspension for safepoints, false if it is running in cooperative mode.
    StackProtectionMode stackProtectionMode;

    // The list of thread contexts attached to this heap
    JsUtil::DoublyLinkedList<MemProtectThreadContext> threadList;

    // The event used to wake up the thread coordinating the safepoint in cooperative mode.
    HANDLE safepointEvent;

    // The thread id of the thread requesting the safepoint so that the background thread does not suspend it.
    DWORD safepointThreadId;

    // This flag is set in the windows where collection is not allowed.
    bool disallowCollection;

    // During process exit we set this flag to avoid performing collections.
    bool processExiting;

    // Are we forcing collections to run in thread?
    bool forceInThread;

    // The head of the singly-linked list of explicit root sections.
    MemProtectHeapRootSection* headRootSection;

    // The critical section used for all synchronization with this heap.
    CriticalSection criticalSection;

    // The callbacks for the recycler
    MemProtectRecyclerCollectionWrapper recyclerCollectionWrapper;

    // Recycler structures
    PageAllocator::BackgroundPageQueue backgroundPageQueue;         /* TODO: Do we really want this?  Current code path doesn't support without this in the recycler*/
    IdleDecommitPageAllocator pageAllocator;
    Recycler recycler;
};

//////////////////////////////////////////////////////////////////////////////
// MemProtectHeap::ConfigurationLoader implementation
//////////////////////////////////////////////////////////////////////////////

Js::Configuration MemProtectHeap::ConfigurationLoader::s_userConfig;
MemProtectHeap::ConfigurationLoader MemProtectHeap::ConfigurationLoader::configurationLoader;

MemProtectHeap::ConfigurationLoader::ConfigurationLoader()
{
    CmdLineArgsParser parser(nullptr, s_userConfig.flags);

    ConfigParser configParser(s_userConfig.flags, _u("memprotect"));
    configParser.ParseRegistry(parser);

    HMODULE hmod = ::GetModuleHandle(NULL);
    configParser.ParseConfig(hmod, parser);
}

Js::ConfigFlagsTable&
MemProtectHeap::ConfigurationLoader::Flags()
{
    return s_userConfig.flags;
}

//////////////////////////////////////////////////////////////////////////////
// MemProtectHeap implementation
//////////////////////////////////////////////////////////////////////////////

void
MemProtectHeap::SignalSafepoint()
{
    Assert(this->InCooperativeMultipleThreadMode());
    SetEvent(this->safepointEvent);
}

void
MemProtectHeap::NotifyUnroot(RecyclerHeapObjectInfo const& heapObjectInfo)
{
    Assert(this->criticalSection.IsLocked());
#ifdef RECYCLER_STRESS
    if (this->recycler.StressCollectNow())
    {
        return;
    }
#endif

    size_t size = heapObjectInfo.GetSize();
    this->unrootSize += size;
    if (!this->recycler.IsObjectMarked(heapObjectInfo.GetObjectAddress()))
    {
        this->unmarkedUnrootSize += size;
    }

    if (this->CanCollect())
    {
        this->Collect(MemProtectHeap_CollectOnUnrootWithHeuristic);
    }
}

void
MemProtectHeap::NotifyCollectionStart(bool partialCollection)
{
    // Save the unroot size at the start of the collection.
    this->oldUnrootSize = this->unrootSize;
    this->oldUnmarkedUnrootSize = this->unmarkedUnrootSize;

    this->partialCollection = partialCollection;

    // Log ETW event
    LogHeapSize(true /* fromGC */);
}

void
MemProtectHeap::NotifyCollectionEnd()
{
    if (this->partialCollection)
    {
        // Assume we collected everything that was unmarked and unrooted when we started the GC
        this->unrootSize -= this->oldUnmarkedUnrootSize;
    }
    else
    {
        // Assume we collected everything that was unrooted when we started the GC
        this->unrootSize -= this->oldUnrootSize;
    }

    // Everything that was unrooted before is now marked.
    this->unmarkedUnrootSize = 0;

    // Get the current heap size
    size_t currentUsedBytes = this->recycler.GetUsedBytes();

    // Set a new limit based on a fixed amount and some fraction of the current used bytes
    this->unrootLimit = UnrootLimitBase + currentUsedBytes / UnrootLimitHeapFraction;

    // Log ETW event
    LogHeapSize(true /* fromGC */);
}

void
MemProtectHeap::LogHeapSize(bool fromGC)
{
    recycler.LogMemProtectHeapSize(fromGC);
}

BOOL
MemProtectHeap::Collect(MemProtectHeapCollectFlags flags, size_t localUnrootSize, size_t localUnmarkedUnrootSize)
{
    this->unrootSize += localUnrootSize;
    this->unmarkedUnrootSize += localUnmarkedUnrootSize;
    return this->Collect(flags);
}

BOOL
MemProtectHeap::Collect(MemProtectHeapCollectFlags flags)
{
    if (flags == MemProtectHeap_CollectOnUnrootWithHeuristic)
    {
        if (this->recycler.CollectionInProgress())
        {
            return this->recycler.FinishConcurrent<FinishConcurrentOnAllocation>();
        }
        else if (this->unrootSize >= this->unrootLimit)
        {
            if (this->unmarkedUnrootSize * 4 >= this->unrootSize * 3)
            {
                return this->recycler.CollectNow<CollectNowConcurrentPartial>();
            }
            else
            {
                return this->recycler.CollectNow<CollectNowConcurrent>();
            }
        }
        else
        {
            return FALSE;
        }
    }
    else if (flags == MemProtectHeap_CollectWithHeuristic)
    {
        if (this->unrootSize >= ExplicitUnrootLimit)
        {
            return this->recycler.CollectNow<CollectNowConcurrent>();
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        BOOL finishResult = FALSE;
        if (flags & MemProtectHeap_ForceFinishCollect)
        {
            if (this->recycler.CollectionInProgress())
            {
                finishResult = this->recycler.FinishConcurrent<ForceFinishCollection>();
            }
        }

        if (flags & MemProtectHeap_CollectForceConcurrent)
        {
            if (flags & MemProtectHeap_CollectForcePartial)
            {
                return this->recycler.CollectNow<CollectNowConcurrentPartial>();
            }
            else
            {
                return this->recycler.CollectNow<CollectNowConcurrent>();
            }
        }
        else if (flags & MemProtectHeap_CollectForcePartial)
        {
            return this->recycler.CollectNow<CollectNowPartial>();
        }
        else if (flags & MemProtectHeap_CollectForce)
        {
            return this->recycler.CollectNow<CollectNowForceInThread>();
        }
        else
        {
            return finishResult;
        }
    }
}

void
MemProtectHeap::AttachThreadContext(MemProtectThreadContext* threadContext)
{
    Assert(criticalSection.IsLocked());
    Assert(CurrentThreadContext() == nullptr);
    bool first = threadList.IsEmpty();

    // Atomically add to list and set as current, to ensure invariants are consistent.
    threadList.LinkToEnd(threadContext);
    TlsSetValue(threadContextTlsIndex, threadContext);

    if (!this->forceInThread && first)
    {
        StartCollectorThreadsOutsideOfCriticalSection();
    }
}

void
MemProtectHeap::DetachThreadContext(MemProtectThreadContext* threadContext)
{
    Assert(criticalSection.IsLocked());
    Assert(CurrentThreadContext() == threadContext);

    if (!this->forceInThread)
    {
        while (threadList.Head() == threadContext && threadContext->Next() == nullptr)
        {
            // Leave the critical section and stop/synchronize with collection threads.
            StopCollectorThreadsOutsideOfCriticalSection();

            if (threadList.Head() == threadContext && threadContext->Next() == nullptr)
            {
                // We stopped the threads outside the critical section and we are still last.
                break;
            }

            // We are no longer the last thread, so we have to undo the stop.
            StartCollectorThreadsOutsideOfCriticalSection();
        }
    }

    // Atomically add to list and set as current, to ensure invariants are consistent.
    TlsSetValue(threadContextTlsIndex, nullptr);
    threadList.Unlink(threadContext);
}

void
MemProtectHeap::ProtectCurrentThread(MemProtectCooperativeThreadWakeProc wakeProc, void* wakeArgument)
{
    MemProtectThreadContext* threadContext = CurrentThreadContext();
    if (threadContext != nullptr)
    {
        FailFast("Protection already enabled for this thread.");
    }

    AutoCriticalSection autocs(&this->criticalSection);

    AUTO_NO_EXCEPTION_REGION;

    threadContext = AllocatorNewNoThrow(HeapAllocator, HeapAllocator::GetNoMemProtectInstance(), MemProtectThreadContext, this, wakeProc, wakeArgument);

    if (threadContext == nullptr)
    {
        FailFast("Failed to allocate thread context, we must not continue without protection");
    }

    AttachThreadContext(threadContext);
}

HRESULT
MemProtectHeap::UnprotectCurrentThread()
{
    MemProtectThreadContext* threadContext = CurrentThreadContext();

    if (threadContext == nullptr)
    {
        // Unprotect on a thread is nop if not currently protected. Indicate this state to caller for tracking.
        return S_FALSE;
    }

    SafepointAutoCriticalSection autocs(threadContext);
    DetachThreadContext(threadContext);
    // Free the memory of the context and null out the tls slot.
    AllocatorDelete(HeapAllocator, HeapAllocator::GetNoMemProtectInstance(), threadContext);

    return S_OK;
}

void
MemProtectHeap::AddRootSection(MemProtectHeapRootSection* rootSection)
{
    rootSection->next = headRootSection;
    headRootSection = rootSection;
}

bool
MemProtectHeap::RemoveRootSection(MemProtectHeapRootSection* rootSection)
{
    MemProtectHeapRootSection** cursor = &headRootSection;
    while (*cursor)
    {
        if (*cursor == rootSection)
        {
            *cursor = (*cursor)->next;
            return true;
        }
    }
    // We did not find the section
    return false;
}

void
MemProtectHeap::Destroy()
{
    // We expect all threads to have been detached from the heap. Otherwise, if protection has leaked,
    // we can't safely shut down and must also leak.
    Assert(this->threadList.IsEmpty());
    if (this->threadList.IsEmpty())
    {
        AllocatorDelete(HeapAllocator, HeapAllocator::GetNoMemProtectInstance(), this);
    }
}

void
MemProtectHeap::OutOfMemory()
{
    // The recycler should never call this for the MemProtect heap
    Assert(false);
    Js::Throw::FatalInternalError();
}

void
MemProtectHeap::StopAllThreads(bool inThread)
{
    Assert(this->InMultipleThreadMode());

    if (this->threadList.IsEmpty())
    {
        // There are no protected threads to stop
        return;
    }

    MemProtectThreadContext* currentThreadContext = CurrentThreadContext();
    if (currentThreadContext == this->threadList.Head() && this->threadList.Head()->Next() == nullptr)
    {
        // The current thread is the only protected thread so there is nobody to stop
        return;
    }

    if (this->InSuspendMultipleThreadMode())
    {
        if (inThread)
        {
            // Remember the current thread id so we do not suspend it
            this->safepointThreadId = GetCurrentThreadId();

            // We can not suspend from in thread, so we delegate to the concurrent thread
            if (!recycler.RequestConcurrentWrapperCallback())
            {
                FailFast("Concurrent callback failed, safepoint could not be reached");
            }
        }
        else
        {
            // We are being called back from the concurrent collector thread so we can safely suspend
            BeginSuspendProtection();
            for (MemProtectThreadContext* threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
            {
                Assert(threadContext->GetThreadId() != GetCurrentThreadId());
                if (threadContext->GetThreadId() != this->safepointThreadId)
                {
                    // Request suspension for all threads that are not running the collection (note that the current thread is not the safepoint thread).
                    threadContext->RequestSuspension();
                }
            }

            for (MemProtectThreadContext* threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
            {
                Assert(threadContext->GetThreadId() != GetCurrentThreadId());
                if (threadContext->GetThreadId() != this->safepointThreadId)
                {
                    // Ensure all threads are suspended in safe regions
                    threadContext->EnsureSuspended();
                }
            }
            EndSuspendProtection();
        }
        return;
    }

    // Stop all threads cooperatively.
    for (MemProtectThreadContext*threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
    {
        if (threadContext != currentThreadContext)
        {
            threadContext->SetStopRequest();
        }
    }

    bool anyRunning = false;
    for (MemProtectThreadContext*threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
    {
        if (threadContext != currentThreadContext)
        {
            if (threadContext->IsRunning())
            {
                // wake any threads that didn't get to safe point on their own. Assume they're idle or slow.
                threadContext->CallWakeProc();
                anyRunning = true;
            }
        }
    }

    if (anyRunning)
    {
        for (MemProtectThreadContext*threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
        {
            if (threadContext != currentThreadContext)
            {
                while (threadContext->IsRunning())
                {
                    WaitForSingleObject(safepointEvent, INFINITE);
                }
            }
        }
    }
}

void
MemProtectHeap::ResumeAllThreads()
{
    if (this->InSuspendMultipleThreadMode())
    {
        for (MemProtectThreadContext*threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
        {
            if (threadContext->GetThreadId() != GetCurrentThreadId())
            {
                threadContext->ResumeFromSuspension();
            }
        }
    }
}

void
MemProtectHeap::BeginSuspendProtection()
{
    if (!HeapLock(GetProcessHeap()))
    {
        FailFast("Failed to acquire heap lock to begin suspend protection");
    }
}

void
MemProtectHeap::EndSuspendProtection()
{
    if (!HeapUnlock(GetProcessHeap()))
    {
        FailFast("Failed to release heap lock to end suspend protection");
    }
}

size_t
MemProtectHeap::FindRoots(RecyclerScanMemoryCallback& scanMemory)
{
    size_t totalScanned = 0;

    if (this->InMultipleThreadMode())
    {
        // If the current thread is a protected thread, then we need to spill registers and save stack bounds for scanning.
        MemProtectThreadContext* currentThreadContext = this->CurrentThreadContext();
        jmp_buf context;
        if (currentThreadContext != nullptr)
        {
            currentThreadContext->SaveContextAndUpdateBounds(context);
        }

        // We then scan all protected stacks (which may include the current thread)
        totalScanned += this->ScanProtectedStacks(scanMemory);
    }

    // We also scan any explicit (non-stack) root sections.
    for (MemProtectHeapRootSection* rootSection = headRootSection; rootSection; rootSection = rootSection->next)
    {
        scanMemory(rootSection->start, rootSection->size);
        totalScanned += rootSection->size;
    }

    return totalScanned;
}

size_t
MemProtectHeap::ScanProtectedStacks(RecyclerScanMemoryCallback& scanMemory)
{
    Assert(this->criticalSection.IsLocked());
    size_t totalScanned = 0;
    for (MemProtectThreadContext*threadContext = threadList.Head(); threadContext != nullptr; threadContext = threadContext->Next())
    {
        totalScanned += threadContext->ScanStack(scanMemory);
    }
    return totalScanned;
}

void
MemProtectHeap::StartCollectorThreadsOutsideOfCriticalSection()
{
    Assert(criticalSection.IsLocked());
    Assert(!this->disallowCollection);

    // Leave the lock here to avoid possible deadlock with alloc/free from DllMain.
    // It's not safe to GC during this interval, so disable garbage collection temporarily.
    this->disallowCollection = true;
    {
        AutoOutsideOfCriticalSection autoOutsideCs(&this->criticalSection);

        // Start up the concurrent collector thread if we are the first context to attach
        if (!this->recycler.EnableConcurrent(nullptr, true))
        {
            FailFast("Failed to start concurrent collection threads");
        }
    }
    Assert(criticalSection.IsLocked());
    Assert(this->disallowCollection);
    this->disallowCollection = false;
}

void
MemProtectHeap::StopCollectorThreadsOutsideOfCriticalSection()
{
    Assert(this->criticalSection.IsLocked());
    Assert(!this->disallowCollection);

    // Stop the concurrent collector thread if we are the last context to detach (we can still collect in-thread)
    this->recycler.DisableConcurrent();
}

//////////////////////////////////////////////////////////////////////////////
// MemProtectRecyclerCollectionWrapper implementation
//////////////////////////////////////////////////////////////////////////////

void MemProtectRecyclerCollectionWrapper::PreCollectionCallBack(CollectionFlags flags)
{
    this->memProtectHeap->NotifyCollectionStart((flags & CollectMode_Partial) == CollectMode_Partial);
}

void MemProtectRecyclerCollectionWrapper::PreRescanMarkCallback()
{
    Assert(this->memProtectHeap->CanCollect());
    if (this->memProtectHeap->InMultipleThreadMode())
    {
        // We are about to perform a rescan so we have to stop concurrent mutation.
        Assert(this->memProtectHeap->CanCollect());
        Assert(!this->inSafepointRegion);
        this->memProtectHeap->StopAllThreads(true);
        this->inSafepointRegion = true;
    }
}

void MemProtectRecyclerCollectionWrapper::RescanMarkTimeoutCallback()
{
    Assert(this->memProtectHeap->CanCollect());
    if (this->memProtectHeap->InMultipleThreadMode())
    {
        // We timed out performing the rescan (try again later)
        Assert(this->inSafepointRegion);
        this->memProtectHeap->ResumeAllThreads();
        this->inSafepointRegion = false;
    }
}

void MemProtectRecyclerCollectionWrapper::EndMarkCallback()
{
    Assert(this->memProtectHeap->CanCollect());
    if (this->memProtectHeap->InMultipleThreadMode())
    {
        // Sweeping means we have finished marking so we can safely resume allocation/mutation.
        Assert(this->inSafepointRegion);
        this->memProtectHeap->ResumeAllThreads();
        this->inSafepointRegion = false;
    }
}

size_t MemProtectRecyclerCollectionWrapper::RootMarkCallback(RecyclerScanMemoryCallback& scanMemory, BOOL* stacksScannedByRuntime)
{
    Assert(this->memProtectHeap->CanCollect());

    if (this->memProtectHeap->InMultipleThreadMode())
    {
        // We are performing the root scan so we have to stop concurrent mutation.
        if (!this->inSafepointRegion)
        {
            // Note that we may already be in a safepoint due to ordering between rescanmark and rootmark
            this->memProtectHeap->StopAllThreads(true);
            this->inSafepointRegion = true;
        }

        // We will scan the stacks (so the recycler should not)
        *stacksScannedByRuntime = true;
    }
    else
    {
        // In single threaded mode we let the recycler scan the current stack.
        *stacksScannedByRuntime = false;
    }

    // Perform root scanning.
    return this->memProtectHeap->FindRoots(scanMemory);
}

void MemProtectRecyclerCollectionWrapper::ConcurrentCallback()
{
    // We are being called back by the concurrent thread
    Assert(this->memProtectHeap->InSuspendMultipleThreadMode());
    Assert(!this->inSafepointRegion);
    this->memProtectHeap->StopAllThreads(false);
}

void MemProtectRecyclerCollectionWrapper::PostCollectionCallBack()
{
    this->memProtectHeap->NotifyCollectionEnd();
}

BOOL MemProtectRecyclerCollectionWrapper::ExecuteRecyclerCollectionFunction(Recycler* recycler, CollectionFunction function, CollectionFlags flags)
{
    if (!this->memProtectHeap->CanCollect())
    {
        // We are not allowed to perform collection work now
        return FALSE;
    }
    BOOL ret = FALSE;
    BEGIN_NO_EXCEPTION
    {
        ret = (recycler->*(function))(flags);
    }
    END_NO_EXCEPTION;
    return ret;
}

void
MemProtectRecyclerCollectionWrapper::DisposeObjects(Recycler* recycler)
{
    if (!this->memProtectHeap->CanCollect())
    {
        // We are not allowed to perform collection work now
        return;
    }
    BEGIN_NO_EXCEPTION
    {
        recycler->DisposeObjects();
    }
    END_NO_EXCEPTION;
}

//////////////////////////////////////////////////////////////////////////////
// MemProtectThreadContext implementation
//////////////////////////////////////////////////////////////////////////////

void
MemProtectThreadContext::RequestSuspension()
{
    if (this->IsLeakedThread())
    {
        // This thread has exited prematurely
        return;
    }


    DWORD suspendCount = SuspendThread(this->hThread);
    if (suspendCount == (DWORD)-1)
    {
        // Suspend may fail here if the thread has exited in the window above.
        Assert(this->IsLeakedThread());
        return;
    }
}

void
MemProtectThreadContext::EnsureSuspended()
{
    if (this->IsLeakedThread())
    {
        return;
    }

    ULONGLONG startTime = GetTickCount64();
    unsigned int attempt = 0;
    while (true)
    {
        //
        // GetThreadContext for a suspended thread has two cases where we can get torn contexts.
        //
        //   1) If we are in the kernel handling an exception then we could see a torn state between
        //      the state when the exception occured and the state we are to resume from after the exception.
        //      For conservative tracing this is benign because we reserve stack space and copy the original
        //      context first.
        //
        //   2) In WoW, we can get the context while running 64 bit code we always just return the saved 32 bit
        //      context for the thread. But we could be in the process of saving it, so we actually see the
        //      context for the previous syscall.
        //
        // On newer OSes (Win8+) we can detect 2 by using CONTEXT_EXCEPTION_REQUEST and the OS will
        // tell us if we are in these dangerous regions (CONTEXT_SERVICE_ACTIVE | CONTEXT_EXCEPTION_ACTIVE).
        //
        // This catches both 1 and 2, but unfortunately any time a thread is blocked the flag will be set so
        // we can not use it without a runtime that is more controlled about leaving "runtime" code.
        //
        //

        // We only care about integer registers and the stack pointer.
        this->context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

        // Get the context from the thread
        if (GetThreadContext(this->hThread, &this->context))
        {
            // Check the thread local state after GetThreadContext (SuspendThread is asynchronous)
            if (!this->inNoSuspendRegion)
            {
                // We are suspended in a safe region.
                break;
            }
        }
        else if (this->IsLeakedThread())
        {
            // A thread has exited between the suspend request and the GetThreadContext call.
            break;
        }

        // We have either suspended in a dangerous region or a region where GetThreadContext fails.
        // On AMD64 and ARM getting the user mode context requires unwinding the stack, which
        // may not be possible due to bad assembly code.

        bool resumedThread = false;

        do
        {
            // Fail if the whole process takes a very long time (1000ms)
            ULONGLONG totalTime = GetTickCount64() - startTime;
            if (totalTime > 1000)
            {
                FailFast("Failed to safely suspend within the required timeframe.")
            }

            if (!resumedThread)
            {
                // Resume the thread
                this->ResumeFromSuspension();
                resumedThread = true;
            }

            // Count this attempt
            attempt++;

            // Try to give the target a chance to run to clear the problem region of code.
            if (totalTime > 10)
            {
                // Sleep for a longer time to allow the other thread to make progress.
                // This will effectively sleep until the next global timer tick (by default 64Hz or 15.625ms)
                Sleep(1);
            }
            else if ((AutoSystemInfo::Data.GetNumberOfPhysicalProcessors() > 1) && ((attempt % 20) != 0))
            {
                // On a multiprocessor machine we simply pause and retry some number of times first
                YieldProcessor();
            }
            else
            {
                // Give up our quantum to allow the other thread to run on this processor.
                SwitchToThread();
            }
        } while (this->inNoSuspendRegion);

        // Re-suspend the thread
        this->RequestSuspension();
    }
}

void
MemProtectThreadContext::ResumeFromSuspension()
{
    if (this->IsLeakedThread())
    {
        return;
    }

    DWORD suspendCount = ResumeThread(this->hThread);
    if (suspendCount == (DWORD)-1)
    {
        // We only resume after Suspend+GetThreadContext has occurred, so there is no race here
        FailFast("ResumeThread failed");
    }
}

MemProtectThreadContext::MemProtectThreadContext(MemProtectHeap* memProtectHeap, MemProtectCooperativeThreadWakeProc wakeProc, void* wakeArgument) :
    inNoSuspendRegion(false),
    disableStackScanningDueToLeakedThread(false),
    state(ThreadState::Protected),
    memProtectHeap(memProtectHeap),
    stackTop(nullptr),
    stackBase(nullptr),
    wakeProc(wakeProc),
    wakeArgument(wakeArgument),
    localUnrootSize(0),
    localUnmarkedUnrootSize(0),
    tib((NT_TIB*)NtCurrentTeb())
{
    DWORD handlePermissions = SYNCHRONIZE;
    if (this->memProtectHeap->InSuspendMultipleThreadMode())
    {
        handlePermissions |= THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION;
    }
    // With suspension we need a handle to stop this thread.
    if (!DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),
        GetCurrentProcess(),
        &this->hThread,
        handlePermissions,
        FALSE,
        0))
    {
        FailFast("Could not create a thread handle (used for SuspendThread)");
    }

    this->threadId = GetCurrentThreadId();
}

MemProtectThreadContext::~MemProtectThreadContext()
{
    CloseHandle(this->hThread);
}

void
MemProtectThreadContext::CallWakeProc()
{
    if (this->wakeProc)
    {
        this->wakeProc(this->wakeArgument);
    }
}

CriticalSection*
MemProtectThreadContext::SafepointEnterCriticalSection()
{
    // If we were in a no-suspend region we are no longer once we acquire the lock
    this->inNoSuspendRegion = false;

    CriticalSection* criticalSection = this->memProtectHeap->GetCriticalSection();

    if (!criticalSection->TryEnter())
    {
        if (!this->memProtectHeap->InSuspendMultipleThreadMode())
        {
            // In cooperative we need to make ourselves safe to allow the collector to progress
            jmp_buf context;
            this->SaveContextAndUpdateBounds(context);

            ThreadState previous = (ThreadState)InterlockedExchange(&(LONG&)this->state, (LONG)ThreadState::Stopped);
            if (previous == ThreadState::StopRequest)
            {
                // Inform pending GC that this thread will wait at a safepoint until the the GC lock is acquired.
                this->memProtectHeap->SignalSafepoint();
            }
        }

        // Actually acquire the lock (and possibly block)
        criticalSection->Enter();

        this->state = ThreadState::Protected;
        this->stackTop = nullptr;
    }

    return criticalSection;
}

void
MemProtectThreadContext::NotifyUnroot(RecyclerHeapObjectInfo const& heapObjectInfo)
{
#ifdef RECYCLER_STRESS
    {
        SafepointAutoCriticalSection autocs(this);
        if (this->GetRecycler()->StressCollectNow())
        {
            return;
        }
    }
#endif

    size_t size = heapObjectInfo.GetSize();
    this->localUnrootSize += size;
    if (!this->GetRecycler()->IsObjectMarked(heapObjectInfo.GetObjectAddress()))
    {
        this->localUnmarkedUnrootSize += size;
    }
    if (this->localUnrootSize >= LocalUnrootReportFrequency)
    {
        this->Collect(MemProtectHeap_CollectOnUnrootWithHeuristic);
    }

    if (heapObjectInfo.IsPageHeapAlloc())
    {
        heapObjectInfo.PageHeapLockPages();
    }
}

BOOL
MemProtectThreadContext::Collect(MemProtectHeapCollectFlags flags)
{
    // To collect we simply take the heap lock and delegate
    SafepointAutoCriticalSection autocs(this);

    // This overload will transfer the statistics to the global heap.
    BOOL result = this->memProtectHeap->Collect(flags, localUnrootSize, localUnmarkedUnrootSize);
    localUnrootSize = 0;
    localUnmarkedUnrootSize = 0;
    return result;
}

void MemProtectThreadContext::SaveContextAndUpdateBounds(jmp_buf& context)
{
    if (this != nullptr)
    {
        Assert(this->threadId == GetCurrentThreadId());
#pragma warning(push)
#pragma warning(disable:4611) // setjmp is not compatible with C++ object destruction, but we'll never call longjmp. We just want the buffer filled with register values.
        setjmp(context); // Use setjmp to get callee save values into buffer.
#pragma warning(pop)
        this->stackTop = (void**)&context;
        this->stackBase = this->tib->StackBase;
    }
}

size_t
MemProtectThreadContext::ScanStack(RecyclerScanMemoryCallback& scanMemory)
{
    if (this->IsLeakedThread())
    {
        return 0;
    }

    size_t contextSize = 0;

    if (this->memProtectHeap->InSuspendMultipleThreadMode())
    {
        if (this->threadId != GetCurrentThreadId())
        {
            Assert(!this->inNoSuspendRegion);

            //
            // Find the stack bounds
            // Note: GetThreadContext has already loaded the context into this->context in EnsureSuspended()
            //

            //
            // The stack pointer and the two addresses in the Teb can be inconsistent during fiber switch,
            // so we can only safely use a single value and VirtualQuery to determine the stack limits.
            // If the stack pointer points within this range we reduce the region scanned down to the sp.
            //

            this->stackTop = this->tib->StackLimit;

#if defined(_M_IX86)
            void* sp = (void*)(this->context.Esp);
#elif defined(_M_X64)
            void* sp = (void*)(this->context.Rsp);
#elif defined(_M_ARM)
            void* sp = (void*)(this->context.Sp);
#elif defined(_M_ARM64)
            void* sp = (void*)(this->context.Sp);
#else
#error Architecture not supported
#endif

            // The protected thread could be in the middle of an operation that has
            // left it's stack not aligned to the pointer size. So round it to the next
            // highest aligned pointer value
            // An example of a case where this could happen is VC++'s implementation of
            // __libm_sse2_exp in VS2015.
            sp = (void*)Math::Align<uintptr_t>((uintptr_t)sp, sizeof(void*));

            // Use the stack top to discover the stack allocation.
            MEMORY_BASIC_INFORMATION memoryInfo;
            if (VirtualQuery(this->stackTop, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
            {
                // We are somehow confused about the stack, which should never happen (fail-fast)
                FailFast("VirtualQuery to find stack extents failed.");
            }

            if ((memoryInfo.Protect & PAGE_GUARD) != 0)
            {
                // Skip over any protected regions at the top of the stack
                void* allocationBase = memoryInfo.AllocationBase;
                while (memoryInfo.AllocationBase == allocationBase && (memoryInfo.Protect & PAGE_GUARD) != 0)
                {
                     if(memoryInfo.RegionSize==0)
                    {
                        // We get zero sized memory region, which should never happen (fail-fast)
                        FailFast("Zero size memory region found in stack memory");
                    }

                    this->stackTop = (void*)((size_t)(this->stackTop) + memoryInfo.RegionSize);
                    if (VirtualQuery(this->stackTop, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
                    {
                        // We are somehow confused about the stack, which should never happen (fail-fast)
                        FailFast("VirtualQuery to find stack extents failed.");
                    }
                }
                if(memoryInfo.AllocationBase != allocationBase)
                {
                    // We failed to get a unprotected memory region in the stack to scan the stack memory,
                    //which should never happen (fail-fast)
                    FailFast("we failed to get a unprotected memory. Fail-fast for better diagnose experience");
                }

            }

            // Set the stack base so we can scan the entire stack if we do not have a valid bottom
            this->stackBase = (void*)((size_t)(memoryInfo.BaseAddress) + memoryInfo.RegionSize);

            // Shorten the range we scan down from the base. This does not protect deliberate
            // esp changes on x86, but if someone can set esp then we have bigger problems.
            if ((sp > this->stackTop) && (sp < this->stackBase))
            {
                // Stack pointer is in the stack bounds so we use it instead.
                this->stackTop = sp;
            }

            // Scan the entire context record
            scanMemory((void**)&this->context, sizeof(CONTEXT));
            contextSize = sizeof(CONTEXT);
        }
    }
    else
    {
        Assert(this->state == ThreadState::Stopped || this->threadId == GetCurrentThreadId());
    }

    Assert(this->stackTop != nullptr);
    Assert(this->stackBase != nullptr);

    // Calculate the stack size
    size_t stackSize = (char*)this->stackBase - (char*)this->stackTop;

    // Scan the actual stack
    scanMemory((void**)this->stackTop, stackSize);

    return stackSize + contextSize;
}

Recycler*
MemProtectThreadContext::GetRecycler()
{
    return this->memProtectHeap->GetRecycler();
}

//////////////////////////////////////////////////////////////////////////////
// External Interface implementation
//////////////////////////////////////////////////////////////////////////////

HRESULT __stdcall MemProtectHeapCreate(void** heapHandle, MemProtectHeapCreateFlags flags)
{
    HRESULT hr = S_OK;
    try
    {
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
        MemProtectHeap* heap = AllocatorNew(HeapAllocator, HeapAllocator::GetNoMemProtectInstance(), MemProtectHeap, flags);
        if (heap == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        *heapHandle = heap;
    }
    catch (Js::OutOfMemoryException)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

template <bool leaf, typename TContext>
__inline void*
MemProtectHeapRootAllocImpl(TContext* context, size_t size)
{
    AUTO_NO_EXCEPTION_REGION;

    byte* buffer = leaf
        ? AllocatorNewNoThrowArrayBase(Recycler, context->GetRecycler(), AllocImplicitRootLeaf, byte, size)
        : AllocatorNewNoThrowArrayBase(Recycler, context->GetRecycler(), AllocImplicitRoot, byte, size);

    JS_ETW(EventWriteMEMPROTECT_OBJECT_ALLOCATION(context->GetRecycler(), size, buffer, MEMPROTECT_HEAP_SOURCE));


    return buffer;
}

template <typename TContext>
__inline HRESULT
MemProtectHeapMemSizeImpl(TContext* context, void* memory, size_t* outSize)
{
    RecyclerHeapObjectInfo heapObject;

    if (!context->GetRecycler()->FindImplicitRootObject(memory, heapObject))
    {
        return E_INVALIDARG;
    }

    if (!heapObject.IsImplicitRoot())
    {
        return E_FAIL;
    }

    *outSize = heapObject.GetSize();
    return S_OK;
}

template <bool leaf, typename TContext>
__inline HRESULT
MemProtectHeapRootReallocImpl(TContext* context, size_t newSize, void** memory)
{
    RecyclerHeapObjectInfo heapObject;
    void* oldMemory = *memory;

    if (!context->GetRecycler()->FindImplicitRootObject(oldMemory, heapObject))
    {
        return E_INVALIDARG;
    }

    size_t objSize = heapObject.GetSize();
    Assert(objSize == HeapInfo::GetAlignedSizeNoCheck(objSize) || heapObject.IsPageHeapAlloc());

    if (leaf != heapObject.IsLeaf())
    {
        // looks in trident there's lots of small allocations (less than pointer size) are allocated as non-leaf
        // in page heap I forced those allocation to be leaf
        if ((!heapObject.IsPageHeapAlloc() && objSize < sizeof(void*)))
        {
            return E_INVALIDARG;
        }
    }

    size_t newSizeAligned = HeapInfo::GetAlignedSizeNoCheck(newSize);
    if (newSizeAligned == objSize)
    {
        // reuse allocation if the new allocation would end up being the same size anyway.
        return S_OK;
    }

    void* buffer = MemProtectHeapRootAllocImpl<leaf>(context, newSize);
    if (buffer == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    memcpy(buffer, oldMemory, min(newSize, objSize));

    if (!leaf)
    {
        memset(oldMemory, 0, objSize);
    }

    if (!heapObject.ClearImplicitRootBit())
    {
        return E_FAIL;
    }
    *memory = buffer;

    JS_ETW(EventWriteMEMPROTECT_OBJECT_UNROOT(context->GetRecycler(), oldMemory, MEMPROTECT_HEAP_SOURCE));

    context->NotifyUnroot(heapObject);
    return S_OK;
}

// Suppress this error since in the case where TContext is MemProtectHeap instead of a threadcontext
// the CS is a void* that should get optimized away
#pragma warning(push)
#pragma warning(disable: 4189)
template <typename TClearImplicitRootBitAutoCriticalSection, typename TContext>
__inline HRESULT
MemProtectHeapBeginUnrootAndZeroImplInternal(TContext* context, void* memory)
{
    RecyclerHeapObjectInfo heapObject;
    Recycler* recycler = context->GetRecycler();

    if (!recycler->FindImplicitRootObject(memory, heapObject))
    {
        return E_INVALIDARG;
    }

    // Zero pointers in order to eliminate false-positives
    if (!heapObject.IsLeaf())
    {
        memset(memory, 0, heapObject.GetSize());
    }

#ifdef RECYCLER_PAGE_HEAP
    // This can be used to hold a critical section (or will be optimized away in most cases)
    TClearImplicitRootBitAutoCriticalSection autocs(context);
#endif

    if (!heapObject.ClearImplicitRootBit())
    {
        return E_FAIL;
    }

    JS_ETW(EventWriteMEMPROTECT_OBJECT_UNROOT(context->GetRecycler(), memory, MEMPROTECT_HEAP_SOURCE));

    context->NotifyUnroot(heapObject);

    return S_OK;
}
#pragma warning(pop)

__inline HRESULT
MemProtectHeapUnrootAndZeroImpl(MemProtectHeap* heap, void* memory)
{
    return MemProtectHeapBeginUnrootAndZeroImplInternal<void*>(heap, memory);
}

__inline HRESULT
MemProtectHeapUnrootAndZeroImpl(MemProtectThreadContext* threadContext, void* memory)
{
#ifdef RECYCLER_PAGE_HEAP
    if (threadContext->GetRecycler()->ShouldCapturePageHeapFreeStack())
    {
        // Capturing the free stack requires allocating memory using the process heap
        // If we're in the process of suspending, this could lead to a deadlock since
        // the GC thread could hold a lock on the process heap
        return MemProtectHeapBeginUnrootAndZeroImplInternal<SafepointAutoCriticalSection>(threadContext, memory);
    }
    else
#endif
    {
        return MemProtectHeapBeginUnrootAndZeroImplInternal<void*>(threadContext, memory);
    }
}

template <typename TContext>
__inline HRESULT
MemProtectHeapRootFreeImpl(TContext* context, void* memory)
{
    RecyclerHeapObjectInfo heapObject;
    if (!context->GetRecycler()->FindImplicitRootObject(memory, heapObject))
    {
        return E_INVALIDARG;
    }

    if (!heapObject.ClearImplicitRootBit())
    {
        return E_FAIL;
    }

    heapObject.ExplicitFree();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Entrypoint template and macro definition
//////////////////////////////////////////////////////////////////////////////

#define MemProtectAlwaysLockedEntrypoint(heapHandle, expression) MemProtectEntrypointBase(true, heapHandle, expression)
#define MemProtectEntrypoint(heapHandle, expression) MemProtectEntrypointBase(false, heapHandle, expression)

// Initially, this macro expanded into a call to a function that did most of the work. However, we found
// that the function, while easier to debug, was inhibiting effective inlining of the Impl methods as
// the lambda could not be marked force-inline. This results in extra call prolog/epilog without any code
// savings (after constant folding, these functions are called from precisely one location each).
#define MemProtectEntrypointBase(alwaysLocked, heapHandle, statement) \
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle); \
    MemProtectThreadContext* threadContext = heap->CurrentThreadContext(); \
    \
if (threadContext != nullptr) \
{ \
    auto context = threadContext; \
if (alwaysLocked) \
{ \
    SafepointAutoCriticalSection autocs(threadContext); \
    statement; \
} \
        else \
{ \
    statement; \
} \
} \
    else \
{ \
    auto context = heap; \
    AutoCriticalSection autocs(heap->GetCriticalSection()); \
    statement; \
}

//////////////////////////////////////////////////////////////////////////////
// External entrypoints
//////////////////////////////////////////////////////////////////////////////

void* __stdcall MemProtectHeapRootAlloc(void* heapHandle, size_t size)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return MemProtectHeapRootAllocImpl<false>(context, size));
}

void* __stdcall MemProtectHeapRootAllocLeaf(void* heapHandle, size_t size)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return MemProtectHeapRootAllocImpl<true>(context, size));
}

HRESULT __stdcall MemProtectHeapRootRealloc(void* heapHandle, size_t newSize, void** memory)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return MemProtectHeapRootReallocImpl<false>(context, newSize, memory));
}

HRESULT __stdcall MemProtectHeapRootReallocLeaf(void* heapHandle, size_t newSize, void** memory)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return MemProtectHeapRootReallocImpl<true>(context, newSize, memory));
}

HRESULT __stdcall MemProtectHeapUnrootAndZero(void* heapHandle, void* memory)
{
    MemProtectEntrypoint(heapHandle, return MemProtectHeapUnrootAndZeroImpl(context, memory));
}

HRESULT __stdcall MemProtectHeapRootFree(__in void* heapHandle, __in void* memory)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return MemProtectHeapRootFreeImpl(context, memory));
}

HRESULT __stdcall MemProtectHeapMemSize(void* heapHandle, void* memory, size_t* outSize)
{
    MemProtectEntrypoint(heapHandle, return MemProtectHeapMemSizeImpl(context, memory, outSize));
}

void __stdcall MemProtectHeapReportHeapSize(__in void* heapHandle)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, return context->GetRecycler()->LogMemProtectHeapSize(false));
}

HRESULT __stdcall MemProtectHeapDestroy(void*heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    heap->Destroy();
    return S_OK;
}

HRESULT __stdcall MemProtectHeapCollect(void* heapHandle, MemProtectHeapCollectFlags flags)
{
    MemProtectEntrypoint(heapHandle, return context->Collect(flags) ? S_OK : S_FALSE);
}

HRESULT __stdcall MemProtectHeapProtectCurrentThread(__in void* heapHandle, __in MemProtectCooperativeThreadWakeProc wakeProc, __in void* wakeArgument)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    if (!heap->InMultipleThreadMode())
    {
        return E_FAIL;
    }
    heap->ProtectCurrentThread(wakeProc, wakeArgument);
    return S_OK;
}

HRESULT __stdcall MemProtectHeapUnprotectCurrentThread(__in void* heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    if (!heap->InMultipleThreadMode())
    {
        return E_FAIL;
    }
    return heap->UnprotectCurrentThread();
}

void __stdcall MemProtectHeapNotifyCurrentThreadDetach(__in void* heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    if (!heap->InMultipleThreadMode())
    {
        return;
    }

    MemProtectThreadContext* currentThreadContext = heap->CurrentThreadContext();
    if (currentThreadContext)
    {
        SafepointAutoCriticalSection autocs(currentThreadContext);

        currentThreadContext->NotifyThreadDetach();
    }
}

HRESULT __stdcall MemProtectHeapSynchronizeWithCollector(void* heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    MemProtectThreadContext* threadContext = heap->CurrentThreadContext();
    if (threadContext != nullptr)
    {
        SafepointAutoCriticalSection autocs(threadContext);
    }
    return S_OK;
}

void __stdcall MemProtectHeapDisableCollection(void* heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    heap->DisableCollectionForProcessExit();
}

void __stdcall MemProtectHeapAddRootSection(__in void* heapHandle, __in MemProtectHeapRootSection* rootSection)
{
    MemProtectAlwaysLockedEntrypoint(heapHandle, ((void)context, static_cast<MemProtectHeap*>(heapHandle)->AddRootSection(rootSection)));
}

HRESULT __stdcall MemProtectHeapRemoveRootSection(__in void* heapHandle, __in MemProtectHeapRootSection* rootSection)
{
    bool removed;
    MemProtectAlwaysLockedEntrypoint(heapHandle, ((void)context, removed = static_cast<MemProtectHeap*>(heapHandle)->RemoveRootSection(rootSection)));
    if (!removed)
    {
        return E_FAIL;
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Testing hooks
//////////////////////////////////////////////////////////////////////////////

// Determine if an allocation is still live.
bool MemProtectHeapIsValidObject(__in void* heapHandle, __in void* memory)
{
    RecyclerHeapObjectInfo heapObject;
    MemProtectAlwaysLockedEntrypoint(heapHandle, return context->GetRecycler()->FindHeapObject(memory, FindHeapObjectFlags_NoFlags, heapObject));
}

#if DBG && defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)
void MemProtectHeapSetDisableConcurrentThreadExitedCheck(void* heapHandle)
{
    MemProtectHeap* heap = reinterpret_cast<MemProtectHeap*>(heapHandle);
    heap->SetDisableConcurentThreadExitedCheck();
}
#endif

#if DBG && defined(INTERNAL_MEM_PROTECT_HEAP_CMDLINE)
void MemProtectHeapParseCmdLineConfig(int argc, __in_ecount(argc) LPWSTR argv[])
{
    MemProtectHeap::ParseCommandLine(argc, argv);
}
#endif
