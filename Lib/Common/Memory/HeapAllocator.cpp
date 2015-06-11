/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

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
#pragma init_seg(".CRT$XCAT")

#ifdef HEAP_TRACK_ALLOC
CriticalSection HeapAllocator::cs;
#endif

#ifdef CHECK_MEMORY_LEAK
MemoryLeakCheck MemoryLeakCheck::leakCheck;
#endif

HeapAllocator HeapAllocator::Instance;
NoThrowHeapAllocator NoThrowHeapAllocator::Instance;
NoCheckHeapAllocator NoCheckHeapAllocator::Instance;
HANDLE NoCheckHeapAllocator::processHeap = NULL;

#ifdef TRACK_ALLOC
#ifdef HEAP_TRACK_ALLOC
__declspec(thread) TrackAllocData HeapAllocator::nextAllocData;
#endif

HeapAllocator * HeapAllocator::TrackAllocInfo(TrackAllocData const& data)
{
#ifdef HEAP_TRACK_ALLOC
    Assert(nextAllocData.IsEmpty());
    nextAllocData = data;
#endif
    return this;
}

void HeapAllocator::ClearTrackAllocInfo(TrackAllocData* data/* = NULL*/)
{
#ifdef HEAP_TRACK_ALLOC
    Assert(!nextAllocData.IsEmpty());
    if (data)
    {
        *data = nextAllocData;
    }
    nextAllocData.Clear();
#endif
}
#endif

#ifdef HEAP_TRACK_ALLOC
//static 
bool HeapAllocator::CheckLeaks()
{
    return Instance.data.CheckLeaks();
}
#endif HEAP_TRACK_ALLOC

char * NoThrowHeapAllocator::AllocZero(size_t byteSize)
{
    return HeapAllocator::Instance.NoThrowAllocZero(byteSize);
}

char * NoThrowHeapAllocator::Alloc(size_t byteSize)
{
    return HeapAllocator::Instance.NoThrowAlloc(byteSize);
}

void NoThrowHeapAllocator::Free(void * buffer, size_t byteSize)
{
    HeapAllocator::Instance.Free(buffer, byteSize);
}

#ifdef TRACK_ALLOC
NoThrowHeapAllocator * NoThrowHeapAllocator::TrackAllocInfo(TrackAllocData const& data) 
{ 
    HeapAllocator::Instance.TrackAllocInfo(data); 
    return this; 
}
#endif TRACK_ALLOC

#ifdef TRACK_ALLOC
void NoThrowHeapAllocator::ClearTrackAllocInfo(TrackAllocData* data /*= NULL*/) 
{ 
    HeapAllocator::Instance.ClearTrackAllocInfo(data); 
}
#endif TRACK_ALLOC

HeapAllocator * HeapAllocator::GetNoMemProtectInstance()
{
#ifdef INTERNAL_MEM_PROTECT_HEAP_ALLOC
    // Used only in Chakra, no need to use CUSTOM_CONFIG_FLAG
    if (CONFIG_FLAG(MemProtectHeap))
    {
        return &NoMemProtectInstance;
    }
#endif
    return &Instance;
}
#ifdef INTERNAL_MEM_PROTECT_HEAP_ALLOC
HeapAllocator HeapAllocator::NoMemProtectInstance(false);

HeapAllocator::HeapAllocator(bool allocMemProtect) : isUsed(false), memProtectHeapHandle(nullptr), allocMemProtect(allocMemProtect)
{
}

bool HeapAllocator::DoUseMemProtectHeap()
{
    if (!allocMemProtect)
    {
        return false;
    }

    if (memProtectHeapHandle != nullptr)
    {
        return true;    
    }

    DebugOnly(bool wasUsed = isUsed);
    isUsed = true;

    // Flag is used only in Chakra, no need to use CUSTOM_CONFIG_FLAG
    if (CONFIG_FLAG(MemProtectHeap))
    {
        Assert(!wasUsed);
        if (FAILED(MemProtectHeapCreate(&memProtectHeapHandle, MemProtectHeapCreateFlags_ProtectCurrentStack)))
        {
            Assert(false);
        }        
        return true;
    }

    return false;
}

void HeapAllocator::FinishMemProtectHeapCollect()
{
    if (memProtectHeapHandle)
    {
        MemProtectHeapCollect(memProtectHeapHandle, MemProtectHeap_ForceFinishCollect);
        DebugOnly(MemProtectHeapSetDisableConcurrentThreadExitedCheck(memProtectHeapHandle));
    }
}

NoThrowNoMemProtectHeapAllocator NoThrowNoMemProtectHeapAllocator::Instance;

char * NoThrowNoMemProtectHeapAllocator::AllocZero(size_t byteSize)
{
    return HeapAllocator::GetNoMemProtectInstance()->NoThrowAllocZero(byteSize);
}

char * NoThrowNoMemProtectHeapAllocator::Alloc(size_t byteSize)
{
    return HeapAllocator::GetNoMemProtectInstance()->NoThrowAlloc(byteSize);
}

void NoThrowNoMemProtectHeapAllocator::Free(void * buffer, size_t byteSize)
{
    HeapAllocator::GetNoMemProtectInstance()->Free(buffer, byteSize);
}

#ifdef TRACK_ALLOC
NoThrowNoMemProtectHeapAllocator * NoThrowNoMemProtectHeapAllocator::TrackAllocInfo(TrackAllocData const& data)
{
    HeapAllocator::GetNoMemProtectInstance()->TrackAllocInfo(data);
    return this;
}
#endif TRACK_ALLOC

#ifdef TRACK_ALLOC
void NoThrowNoMemProtectHeapAllocator::ClearTrackAllocInfo(TrackAllocData* data /*= NULL*/)
{
    HeapAllocator::GetNoMemProtectInstance()->ClearTrackAllocInfo(data);
}
#endif TRACK_ALLOC
#endif

#if defined(HEAP_TRACK_ALLOC) || defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)

HeapAllocator::~HeapAllocator()
{
#ifdef HEAP_TRACK_ALLOC
    bool hasFakeHeapLeak = false;
    auto fakeHeapLeak = [&]() 
    {
        // REVIEW: Ok to use global flags?
        if (Js::Configuration::Global.flags.ForceMemoryLeak && !hasFakeHeapLeak)
        {
            AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);
            struct FakeMemory { int f; };
            HeapNewStruct(FakeMemory);
            hasFakeHeapLeak = true;
        }
    };

#ifdef LEAK_REPORT
    // REVIEW: Ok to use global flags?
    if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
    {
        fakeHeapLeak();
        LeakReport::StartSection(L"Heap Leaks");
        LeakReport::StartRedirectOutput();
        bool leaked = !HeapAllocator::CheckLeaks();
        LeakReport::EndRedirectOutput();
        LeakReport::EndSection();

        LeakReport::Print(L"--------------------------------------------------------------------------------\n");
        if (leaked)
        {
            LeakReport::Print(L"Heap Leaked Object: %d bytes (%d objects)\n",
                data.outstandingBytes, data.allocCount - data.deleteCount);
        }
    }
#endif // LEAK_REPORT

#ifdef CHECK_MEMORY_LEAK
    // REVIEW: Ok to use global flags?
    if (Js::Configuration::Global.flags.CheckMemoryLeak)
    {
        fakeHeapLeak();
        Output::CaptureStart();
        Output::Print(L"-------------------------------------------------------------------------------------\n");
        Output::Print(L"Heap Leaks\n");
        Output::Print(L"-------------------------------------------------------------------------------------\n");
        if (!HeapAllocator::CheckLeaks())
        {
            Output::Print(L"-------------------------------------------------------------------------------------\n");
            Output::Print(L"Heap Leaked Object: %d bytes (%d objects)\n",
                data.outstandingBytes, data.allocCount - data.deleteCount);
            wchar_t * buffer = Output::CaptureEnd();
            MemoryLeakCheck::AddLeakDump(buffer, data.outstandingBytes, data.allocCount - data.deleteCount);
        }
        else
        {
            free(Output::CaptureEnd());
        }
    }
#endif // CHECK_MEMORY_LEAK
#endif // HEAP_TRACK_ALLOC

#ifdef INTERNAL_MEM_PROTECT_HEAP_ALLOC
    if (memProtectHeapHandle != null)
    {
        MemProtectHeapDestroy(memProtectHeapHandle);
    }
#endif // INTERNAL_MEM_PROTECT_HEAP_ALLOC
}
#endif // defined(HEAP_TRACK_ALLOC) || defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)

#ifdef HEAP_TRACK_ALLOC
void
HeapAllocatorData::LogAlloc(HeapAllocRecord * record, size_t requestedBytes, TrackAllocData const& data)
{
    record->prev = null;
    record->size = requestedBytes;

    record->data = this;
    record->next = head;
    record->allocId = allocCount;
    record->allocData = data;
    if (head != null)
    {
        head->prev = record;
    }
    head = record;
    outstandingBytes += requestedBytes;
    allocCount++;

#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    // REVIEW: Ok to use global flags?
    if (Js::Configuration::Global.flags.LeakStackTrace)
    {
        // Allocation done before the flags is parse doesn't get a stack trace
        record->stacktrace = StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, 1, StackTraceDepth);
    }
    else
    {
        record->stacktrace = null;
    }
#endif
}

void
HeapAllocatorData::LogFree(HeapAllocRecord * record)
{
    Assert(record->data == this);

    // This is an expensive check for double free
#if 0
    HeapAllocRecord * curr = head;
    while (curr != null)
    {
        if (curr == record)
        {
            break;
        }
        curr = curr->next;
    }
    Assert(curr != null);
#endif
    if (record->next != null)
    {
        record->next->prev = record->prev;
    }
    if (record->prev == null)
    {
        head = record->next;
    }
    else
    {
        record->prev->next = record->next;
    }

    deleteCount++;
    outstandingBytes -= record->size;
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    if (record->stacktrace != null)
    {
        record->stacktrace->Delete(&NoCheckHeapAllocator::Instance);
    }
#endif
}

bool
HeapAllocatorData::CheckLeaks()
{
    bool needPause = false;
    if (allocCount != deleteCount)
    {
        needPause = true;

        HeapAllocRecord * current = head;
        while (current != null)
        {
            Output::Print(L"%S%s", current->allocData.GetTypeInfo()->name(),
                current->allocData.GetCount() == (size_t)-1? L"" : L"[]");
            Output::SkipToColumn(50);
            Output::Print(L"- %p - %10d bytes\n",
                ((char*)current) + ::Math::Align<size_t>(sizeof(HeapAllocRecord), MEMORY_ALLOCATION_ALIGNMENT),
                current->size);
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
            // REVIEW: Ok to use global flags?
            if (Js::Configuration::Global.flags.LeakStackTrace && current->stacktrace)
            {
                // Allocation done before the flags is parse doesn't get a stack trace
                Output::Print(L" Allocation Stack:\n");
                current->stacktrace->Print();
            }
#endif
            current = current->next;
        }
    }
    else if (outstandingBytes != 0)
    {
        needPause = true;
        Output::Print(L"Unbalanced new/delete size: %d\n", outstandingBytes);
    }

    Output::Flush();

#if defined(ENABLE_DEBUG_CONFIG_OPTIONS) && !DBG
    // REVIEW: Ok to use global flags?
    if (needPause && Js::Configuration::Global.flags.Console)
    {
        //This is not defined for WinCE
        HANDLE handle = GetStdHandle( STD_INPUT_HANDLE );

        FlushConsoleInputBuffer(handle);

        Output::Print(L"Press any key to continue...\n");
        Output::Flush();

        WaitForSingleObject(handle, INFINITE);

    }
#endif
    return allocCount == deleteCount && outstandingBytes == 0;
}

#endif


#ifdef CHECK_MEMORY_LEAK
MemoryLeakCheck::~MemoryLeakCheck()
{
    if (head != null)
    {
        if (enableOutput)
        {
            Output::Print(L"FATAL ERROR: Memory Leak Detected\n");
        }
        LeakRecord * current = head;
        do
        {

            if (enableOutput)
            {
                Output::PrintBuffer(current->dump, wcslen(current->dump));
            }
            LeakRecord * prev = current;
            current = current->next;
            free((void *)prev->dump);
            NoCheckHeapDelete(prev);
        }
        while (current != null);
        if (enableOutput)
        {
            Output::Print(L"-------------------------------------------------------------------------------------\n");
            Output::Print(L"Total leaked: %d bytes (%d objects)\n", leakedBytes, leakedCount);
            Output::Flush();
        }
    }
}

void
MemoryLeakCheck::AddLeakDump(wchar_t const * dump, size_t bytes, size_t count)
{
    AutoCriticalSection autocs(&leakCheck.cs);
    LeakRecord * record = NoCheckHeapNewStruct(LeakRecord);
    record->dump = dump;
    record->next = null;
    if (leakCheck.tail == null)
    {
        leakCheck.head = record;
        leakCheck.tail = record;
    }
    else
    {
        leakCheck.tail->next = record;
        leakCheck.tail = record;
    }
    leakCheck.leakedBytes += bytes;
    leakCheck.leakedCount += count;
}
#endif
