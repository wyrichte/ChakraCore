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
        bool IsUsingGlobalListFirst();
        bool IsUsingThreadContextBase();
        bool IsUsingTemplatedLinkedList();
    private:
        Nullable<bool> m_usingThreadContextTLSEntry;    // If the build uses ThreadContextTLSEntry::s_tlsEntryList
        Nullable<bool> m_usingThreadContextTLSSlot;     // If the build uses ThreadContextTLSEntry::s_tlsSlot
        Nullable<bool> m_usingTLSEntryList;             // If the build uses chained ThreadContextTLSEntry to walk the thread contexts
        Nullable<bool> m_usingGlobalListFirst;
        Nullable<bool> m_usingThreadContextBase;        // If the build uses ThreadContextBase class. Casting to ThreadContext* involves a bit more work because of multiple inheritence.
        Nullable<bool> m_usingTemplatedLinkedList;
    };
    static Info * GetInfo();
    static RemoteThreadContext GetCurrentThreadContext(ULONG64 fallbackRecyclerAddress = 0);
    static bool TryGetCurrentThreadContext(RemoteThreadContext& remoteThreadContext);
    template <typename Fn>
    static bool ForEach(Fn fn)
    {        
        ExtRemoteTyped container = GetFirstThreadContextContainer();
        while (ExtRemoteTypedUtil::GetAsPointer(container))
        {
            RemoteThreadContext threadContext = GetThreadContextFromContainer(container);

            ULONG64 ptr = threadContext.GetPtr();
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
    bool TryGetDebuggerThreadId(ULONG * pDebuggerThreadId, ULONG * pThreadId = NULL);
    bool UseCodePageAllocator();
    RemoteRecycler GetRecycler();
    ULONG64 GetPtr();

    template <typename Fn>
    bool ForEachScriptContext(Fn fn)
    {
        if (threadContext.HasField("scriptContextList"))
        {
            return ExtRemoteTypedUtil::LinkListForEach(threadContext.Field("scriptContextList"), "next", [&](ExtRemoteTyped& scriptContext)
            {
                return fn(RemoteScriptContext(scriptContext));
            });
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
        if (recycler.GetPtr() == 0)
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
        else if (threadContext.HasField("debugManager"))
        {
            ExtRemoteTyped debugManager = threadContext.Field("debugManager");
            if (debugManager.GetPtr() != 0)
            {
                fn("Diag", RemotePageAllocator(debugManager.Field("diagnosticPageAllocator")));
            }
        }
        
        if (threadContext.HasField("jobProcessor"))
        {
            ExtRemoteTyped jobProcessor = threadContext.Field("jobProcessor");
            if (threadContext.Field("bgJit").GetStdBool() && !threadContext.Field("isOptimizedForManyInstances").GetStdBool())
            {
                ExtRemoteTyped backgroundJobProcessor(GetExtension()->FillModule("(%s!JsUtil::BackgroundJobProcessor *)@$extin)"), jobProcessor.GetPtr());                
                if (backgroundJobProcessor.HasField("maxThreadCount"))
                {
                    uint maxThreadCount = backgroundJobProcessor.Field("maxThreadCount").GetUlong();
                    ExtRemoteTyped parallelThreadData = backgroundJobProcessor.Field("parallelThreadData");
                    for (uint i = 0; i < maxThreadCount; i++)
                    {
                        fn("BGJob", RemotePageAllocator(parallelThreadData.ArrayElement(i).Field("backgroundPageAllocator")));
                    }
                }
                else
                {
                    // IE11 don't have parallel parse
                    fn("BGJob", RemotePageAllocator(backgroundJobProcessor.Field("backgroundPageAllocator")));
                }
            }
        }        

        // Switch to per thread code page allocators in RS1
        bool useCodePageAllocators = this->UseCodePageAllocator();
        if (useCodePageAllocators)
        {
            ExtRemoteTyped codePageAllocators = threadContext.Field("codePageAllocators");
            
            if (codePageAllocators.HasField("preReservedHeapAllocator"))
            {
                // Renamed in commit ac56bc05618bf964cce48213d2d503e88d5f4536: Use Sections for OOP JIT
                fn("CodePreRes", RemotePageAllocator(codePageAllocators.Field("preReservedHeapAllocator")));
            }
            else
            {
                fn("CodePreRes", RemotePageAllocator(codePageAllocators.Field("preReservedHeapPageAllocator")));
            }
            fn("Code", RemotePageAllocator(codePageAllocators.Field("pageAllocator")));

            ExtRemoteTyped thunkPageAllocators = threadContext.Field("thunkPageAllocators");
            if (thunkPageAllocators.HasField("preReservedHeapAllocator"))
            {
                fn("CodeThunkPreRes", RemotePageAllocator(thunkPageAllocators.Field("preReservedHeapAllocator")));
            }
            else
            {
                fn("CodeThunkPreRes", RemotePageAllocator(thunkPageAllocators.Field("preReservedHeapPageAllocator")));
            }
            fn("CodeThunk", RemotePageAllocator(thunkPageAllocators.Field("pageAllocator")));
        }

        this->ForEachScriptContext([fn](RemoteScriptContext scriptContext)
        {
            scriptContext.ForEachPageAllocator(fn);
            return false;
        });
    }
    uint GetCallRootLevel() 
    {
        return threadContext.Field("callRootLevel").GetLong();
    }    
    RemoteInterpreterStackFrame GetLeafInterpreterStackFrame()
    {
        return threadContext.Field("leafInterpreterFrame");
    }

    static bool HasThreadId();
    static bool TryGetThreadContextFromAnyContextPointer(ULONG64 contextPointer, RemoteThreadContext& remoteThreadContext);
    static bool TryGetThreadContextFromPointer(ULONG64 pointer, RemoteThreadContext& remoteThreadContext);
private:
    bool HasThreadIdField();
    ULONG GetThreadId();
    static bool TryGetThreadContextFromTeb(RemoteThreadContext& remoteThreadContext);

    ExtRemoteTyped threadContext;

    static bool IsUsingGlobalListFirst();
    static bool IsUsingThreadContextTLSEntry();
    static bool IsUsingThreadContextTLSSlot();    
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
