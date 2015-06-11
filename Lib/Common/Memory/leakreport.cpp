//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef LEAK_REPORT
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
#pragma init_seg(".CRT$XCAR")

CriticalSection LeakReport::s_cs;
DWORD LeakReport::nestedSectionCount = 0;
DWORD LeakReport::nestedRedirectOutputCount = 0;
AutoFILE LeakReport::file;
FILE * oldFile = null;
bool LeakReport::openReportFileFailed = false;
LeakReport::UrlRecord * LeakReport::urlRecordHead = null;
LeakReport::UrlRecord * LeakReport::urlRecordTail = null;

void
LeakReport::StartRedirectOutput()
{
    if (!EnsureLeakReportFile())
    {
        return;
    }
    s_cs.Enter();
    if (nestedRedirectOutputCount == 0)
    {        
        Assert(oldFile == null);
        oldFile = Output::SetFile(file);
    }
    nestedRedirectOutputCount++;
}

void
LeakReport::EndRedirectOutput()
{
    if (nestedRedirectOutputCount == 0)
    {
        return;
    }
    Assert(file != null);
    nestedRedirectOutputCount--;

    if (nestedRedirectOutputCount == 0)
    {
        fflush(file);
        FILE * tmpFile = Output::SetFile(oldFile);
        Assert(tmpFile == file);
        oldFile = null;
    }
    s_cs.Leave();
}

void
LeakReport::StartSection(wchar_t const * msg, ...)
{     
    va_list argptr;
    va_start(argptr, msg);
    StartSection(msg, argptr);
    va_end(argptr);
}

void
LeakReport::StartSection(wchar_t const * msg, va_list argptr)
{   
    s_cs.Enter();
    if (!EnsureLeakReportFile())
    {
        return;
    }
    nestedSectionCount++;


    Print(L"--------------------------------------------------------------------------------\n");
    vfwprintf(file, msg, argptr);
    Print(L"\n");
    Print(L"--------------------------------------------------------------------------------\n");
}

void
LeakReport::EndSection()
{
    s_cs.Leave();
    if (file == null)
    {
        return;
    }
    nestedSectionCount--;
}

void
LeakReport::Print(wchar_t const * msg, ...)
{
    AutoCriticalSection autocs(&s_cs);
    if (!EnsureLeakReportFile())
    {
        return;
    }

    va_list argptr;
    va_start(argptr, msg);
    vfwprintf(file, msg, argptr);
    va_end(argptr);
}

bool 
LeakReport::EnsureLeakReportFile()
{
    AutoCriticalSection autocs(&s_cs);
    if (openReportFileFailed)
    {
        return false;
    }
    if (file != null)
    {
        return true;
    }

    // REVIEW: Can't think of a clean way to do this
    // Leaving as is so that we just use the default filename
    wchar_t const * filename = Js::Configuration::Global.flags.LeakReport;
    wchar_t const * openMode = L"w+";
    wchar_t defaultFilename[_MAX_PATH];
    if (filename == null)
    {
        swprintf_s(defaultFilename, L"jsleakreport-%d.txt", ::GetCurrentProcessId());
        filename = defaultFilename;
        openMode = L"a+";   // append mode
    }
    if (_wfopen_s(&file, filename, openMode) != 0)
    {
        openReportFileFailed = true;
        return false;
    }
    Print(L"================================================================================\n");
    Print(L"Jscript9 Leak Report - PID: %d\n", ::GetCurrentProcessId());
    time_t time_value = time(NULL);    
    Print(_wasctime(localtime(&time_value)));
    Print(L"\n");
    return true;
}

LeakReport::UrlRecord *
LeakReport::LogUrl(wchar_t const * url, void * globalObject)
{
    UrlRecord * record = NoCheckHeapNewStruct(UrlRecord);

    charcount_t length = wcslen(url) + 1; // Add 1 for the NULL.
    wchar_t* urlCopy = NoCheckHeapNewArray(wchar_t, length);
    js_memcpy_s(urlCopy, (length - 1) * sizeof(wchar_t), url, (length - 1) * sizeof(wchar_t));
    urlCopy[length - 1] = L'\0';

    record->url = urlCopy;
    record->time = time(NULL);
    record->tid = ::GetCurrentThreadId();
    record->next = null;       
    record->scriptEngine = null;
    record->globalObject = globalObject;        // TODO: Switch it to JavascriptLibrary when Yong change to use Library in the type
   
    AutoCriticalSection autocs(&s_cs);
    if (LeakReport::urlRecordHead == null)
    {
        Assert(LeakReport::urlRecordTail == null);
        LeakReport::urlRecordHead = record;
        LeakReport::urlRecordTail = record;        
    }
    else
    {
        LeakReport::urlRecordTail->next = record;
        LeakReport::urlRecordTail = record;
    }

    return record;
}

void
LeakReport::DumpUrl(DWORD tid)
{
    AutoCriticalSection autocs(&s_cs);
    if (!EnsureLeakReportFile())
    {
        return;
    }
        
    UrlRecord * prev = null;
    UrlRecord ** pprev = &LeakReport::urlRecordHead;
    UrlRecord * curr = *pprev;
    while (curr != null)
    {
        if (curr->tid == tid)
        {
            wchar_t * timeStr = _wasctime(localtime(&curr->time));
            timeStr[wcslen(timeStr) - 1] = 0;                  
            Print(L"%s - (%p, %p) %s\n", timeStr, curr->scriptEngine, curr->globalObject, curr->url);
            *pprev = curr->next;   
            NoCheckHeapDeleteArray(wcslen(curr->url) + 1, curr->url);
            NoCheckHeapDelete(curr);
        }
        else
        {
            pprev = &curr->next;
            prev = curr;
        }
        curr = *pprev;
    }   

    if (prev == null)
    {
        LeakReport::urlRecordTail = null;
    }
    else if (prev->next == null)
    {
        LeakReport::urlRecordTail = prev;
    }
}

AutoLeakReportSection::AutoLeakReportSection(Js::ConfigFlagsTable& flags, wchar_t const * msg, ...):
    m_flags(flags)
{
    if (flags.IsEnabled(Js::LeakReportFlag))
    {
        va_list argptr;
        va_start(argptr, msg);
        LeakReport::StartSection(msg, argptr);
        va_end(argptr);
    }
}

AutoLeakReportSection::~AutoLeakReportSection()
{
    if (m_flags.IsEnabled(Js::LeakReportFlag))
    {        
        LeakReport::EndSection();
    }
}
#endif