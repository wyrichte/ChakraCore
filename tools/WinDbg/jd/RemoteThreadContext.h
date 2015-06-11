//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------
#include "RemoteRecycler.h"

class RemoteThreadContext
{
public:
    class Info
    {
    public:
        bool IsUsingThreadContextTLSEntry();
        bool IsUsingThreadContextTLSSlot();
        bool IsUsingTLSEntryList();
        bool IsUsingThreadContextBase();
        bool IsUsingTemplatedLinkedList();
    private:
        Nullable<bool> m_usingThreadContextTLSEntry;    // If the build uses ThreadContextTLSEntry::s_tlsEntryList
        Nullable<bool> m_usingThreadContextTLSSlot;     // If the build uses ThreadContextTLSEntry::s_tlsSlot
        Nullable<bool> m_usingTLSEntryList;             // If the build uses chained ThreadContextTLSEntry to walk the thread contexts
        Nullable<bool> m_usingThreadContextBase;        // If the build uses ThreadContextBase class. Casting to ThreadContext* involves a bit more work because of multiple inheritence.
        Nullable<bool> m_usingTemplatedLinkedList;
    };
    static Info * GetInfo();    
    static RemoteThreadContext GetThreadContextFromTeb(ExtRemoteTyped teb);
    static RemoteThreadContext GetCurrentThreadContext();
    static bool TryGetCurrentThreadContext(RemoteThreadContext& remoteThreadContext);
    template <typename Fn>
    static bool ForEach(Fn fn)
    {        
        ExtRemoteTyped container = GetFirstThreadContextContainer();
        while (GetAsPointer(container))
        {
            RemoteThreadContext threadContext = GetThreadContextFromContainer(container);

            ULONG64 ptr = threadContext.GetExtRemoteTyped().GetPtr();
            if (ptr != 0 && ptr != -1)
            {
                if (fn(threadContext))
                {
                    return true;
                }
            }

            container = GetNextThreadContextContainer(container);
        }
        return false;     
    }

    
    RemoteThreadContext() {}
    RemoteThreadContext(ExtRemoteTyped const& threadContext) : threadContext(threadContext) {};
    ExtRemoteTyped GetExtRemoteTyped() { return threadContext; }
    ULONG GetThreadId();
    RemoteRecycler GetRecycler();

    template <typename Fn>
    bool ForEachScriptContext(Fn fn)
    {
        if (threadContext.HasField("scriptContextList"))
        {
            return LinkListForEach(threadContext.Field("scriptContextList"), "next", fn);
        }
        else
        {
            return false;
        }
    }

    template <typename Fn>
    void ForEachPageAllocator(Fn fn)
    {        
        RemoteRecycler recycler = this->GetRecycler();
        if (recycler.GetExtRemoteTyped().GetPtr() == 0)
        {
            // Recycler not initialized?  Just print the thread one
            fn("Thread", RemotePageAllocator(threadContext.Field("pageAllocator")));            
        }
        else if (threadContext.HasField("leafPageAllocator"))
        {
            fn("Thread", RemotePageAllocator(threadContext.Field("pageAllocator")));            
            recycler.ForEachPageAllocator("RecyclerLeaf", fn);
        }
        else
        {
            recycler.ForEachPageAllocator("Thread", fn);
        }

        if (threadContext.HasField("diagnosticPageAllocator"))
        {
            fn("Diag", RemotePageAllocator(threadContext.Field("diagnosticPageAllocator")));
        }
        
        if (threadContext.HasField("jobProcessor"))
        {
            ExtRemoteTyped jobProcessor = threadContext.Field("jobProcessor");
            if (threadContext.Field("bgJit").GetStdBool() && !threadContext.Field("isOptimizedForManyInstances").GetStdBool())
            {
                ExtRemoteTyped backgroundJobProcessor(GetExtension()->FillModule("(%s!JsUtil::BackgroundJobProcessor *)@$extin)"), jobProcessor.GetPtr());
                uint maxThreadCount = backgroundJobProcessor.Field("maxThreadCount").GetUlong();
                ExtRemoteTyped parallelThreadData = backgroundJobProcessor.Field("parallelThreadData");
                for (uint i = 0; i < maxThreadCount; i++)
                {
                    fn("BGParse", RemotePageAllocator(parallelThreadData.ArrayElement(i).Field("backgroundPageAllocator")));
                }
            }
        }

        auto forEachCodeGenAllocatorPageAllocator = [fn](ExtRemoteTyped codeGenAllocators)
        {
            if (codeGenAllocators.GetPtr() == 0) { return; }

            fn("BGJIT", RemotePageAllocator(codeGenAllocators.Field("pageAllocator")));
            fn("CustomHeap", RemotePageAllocator(codeGenAllocators.Field("emitBufferManager.allocationHeap.pageAllocator")));
        };
        this->ForEachScriptContext([forEachCodeGenAllocatorPageAllocator](ExtRemoteTyped scriptContext)
        {
            ExtRemoteTyped nativeCodeGen = scriptContext.Field("nativeCodeGen");

            forEachCodeGenAllocatorPageAllocator(nativeCodeGen.Field("foregroundAllocators"));
            forEachCodeGenAllocatorPageAllocator(nativeCodeGen.Field("backgroundAllocators"));            
            return false;
        });
    }
private:
    ExtRemoteTyped threadContext;

    static bool IsUsingThreadContextTLSEntry();
    static bool IsUsingThreadContextTLSSlot();
    static bool IsUsingTLSEntryList();
    static bool IsUsingThreadContextBase();
    static bool IsUsingTemplatedLinkedList();
    static bool GetTlsSlot(ExtRemoteTyped& teb, ULONG tlsSlotIndex, ULONG64* pValue);
    static ExtRemoteTyped GetTlsEntryList();
    static ExtRemoteTyped GetFirstThreadContextContainer();
    static ExtRemoteTyped GetNextThreadContextContainer(ExtRemoteTyped container);
    static RemoteThreadContext GetThreadContextFromContainer(ExtRemoteTyped container);

};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------