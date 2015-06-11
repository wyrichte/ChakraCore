/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

#ifdef PROFILE_RECYCLER_ALLOC  
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
#pragma init_seg(".CRT$XCAS")

RecyclerObjectDumper::DumpFunctionMap * RecyclerObjectDumper::dumpFunctionMap = null;
RecyclerObjectDumper RecyclerObjectDumper::Instance;

RecyclerObjectDumper::~RecyclerObjectDumper()
{
    if (dumpFunctionMap)
    {
        NoCheckHeapDelete(dumpFunctionMap);
    }
}

BOOL
RecyclerObjectDumper::EnsureDumpFunctionMap()
{
    if (dumpFunctionMap == null)
    {
        dumpFunctionMap = NoCheckHeapNew(DumpFunctionMap, &NoCheckHeapAllocator::Instance);
    }
    return (dumpFunctionMap != null);
}

void 
RecyclerObjectDumper::RegisterDumper(type_info const * typeinfo, DumpFunction dumperFunction)
{
    if (EnsureDumpFunctionMap())
    {
        Assert(!dumpFunctionMap->HasEntry(typeinfo));
        dumpFunctionMap->Add(typeinfo, dumperFunction);
    }
}

void 
RecyclerObjectDumper::DumpObject(type_info const * typeinfo, bool isArray, void * objectAddress)
{
    if (typeinfo == null)
    {
        Output::Print(L"Address %p", objectAddress);
    }
    else
    {
        DumpFunction dumpFunction;
        if (dumpFunctionMap == null || !dumpFunctionMap->TryGetValue(typeinfo, &dumpFunction) || !dumpFunction(typeinfo, isArray, objectAddress))
        {
            Output::Print(isArray? L"%S[] %p" : L"%S %p", typeinfo->name(), objectAddress);
        }        
    }
}
#endif