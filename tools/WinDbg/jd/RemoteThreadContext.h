//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

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
    bool ForEachPageAllocator(Fn fn)
    {        
        RemoteRecycler recycler = this->GetRecycler();
        if (recycler.GetPtr() == 0)
        {
            // Recycler not initialized?  Just print the thread one
            if (fn("Thread", RemotePageAllocator(threadContext.Field("pageAllocator"))))
            {
                return true;
            }
        }
        else if (threadContext.HasField("leafPageAllocator"))
        {
            if (fn("Thread", RemotePageAllocator(threadContext.Field("pageAllocator")))
                || recycler.ForEachPageAllocator("RecyclerLeaf", fn))
            {
                return true;
            }
        }
        else
        {
            if (recycler.ForEachPageAllocator("Thread", fn))
            {
                return true;
            }
        }

        if (threadContext.HasField("diagnosticPageAllocator"))
        {
            if (fn("Diag", RemotePageAllocator(threadContext.Field("diagnosticPageAllocator"))))
            {
                return true;
            }
        }
        else if (threadContext.HasField("debugManager"))
        {
            ExtRemoteTyped debugManager = threadContext.Field("debugManager");
            if (debugManager.GetPtr() != 0)
            {
                if (fn("Diag", RemotePageAllocator(debugManager.Field("diagnosticPageAllocator"))))
                {
                    return true;
                }
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
                        if (fn("BGJob", RemotePageAllocator(parallelThreadData.ArrayElement(i).Field("backgroundPageAllocator"))))
                        {
                            return true;
                        }
                    }
                }
                else
                {
                    // IE11 don't have parallel parse
                    if (fn("BGJob", RemotePageAllocator(backgroundJobProcessor.Field("backgroundPageAllocator"))))
                    {
                        return true;
                    }
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
                if (fn("CodePreRes", RemotePageAllocator(codePageAllocators.Field("preReservedHeapAllocator"))))
                {
                    return true;
                }
            }
            else
            {
                if (fn("CodePreRes", RemotePageAllocator(codePageAllocators.Field("preReservedHeapPageAllocator"))))
                {
                    return true;
                }
            }
            if (fn("Code", RemotePageAllocator(codePageAllocators.Field("pageAllocator"))))
            {
                return true;
            }

            ExtRemoteTyped thunkPageAllocators = threadContext.Field("thunkPageAllocators");
            if (thunkPageAllocators.HasField("preReservedHeapAllocator"))
            {
                if (fn("CodeThunkPreRes", RemotePageAllocator(thunkPageAllocators.Field("preReservedHeapAllocator"))))
                {
                    return true;
                }
            }
            else
            {
                if (fn("CodeThunkPreRes", RemotePageAllocator(thunkPageAllocators.Field("preReservedHeapPageAllocator"))))
                {
                    return true;
                }
            }
            if (fn("CodeThunk", RemotePageAllocator(thunkPageAllocators.Field("pageAllocator"))))
            {
                return true;
            }
        }

        return this->ForEachScriptContext([fn](RemoteScriptContext scriptContext)
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
