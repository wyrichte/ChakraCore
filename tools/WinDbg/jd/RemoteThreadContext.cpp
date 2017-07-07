//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

#include "RemoteThreadContext.h"

bool RemoteThreadContext::Info::IsUsingGlobalListFirst()
{
    GetExtension()->DetectFeatureBySymbol(m_usingGlobalListFirst, GetExtension()->FillModule("%s!ThreadContext::globalListFirst"));
    return m_usingGlobalListFirst;
}

bool RemoteThreadContext::Info::IsUsingThreadContextTLSEntry()
{
    GetExtension()->DetectFeatureBySymbol(m_usingThreadContextTLSEntry, GetExtension()->FillModule("%s!ThreadContextTLSEntry::s_tlsEntryList"));
    return m_usingThreadContextTLSEntry;
}

bool RemoteThreadContext::Info::IsUsingThreadContextTLSSlot()
{
    // Newer builds use ThreadContextTLSEntry::s_tlsSlot, older use ThreadContext::s_tlsSlot
    GetExtension()->DetectFeatureBySymbol(m_usingThreadContextTLSSlot, GetExtension()->FillModule("%s!ThreadContextTLSEntry::s_tlsSlot"));
    return m_usingThreadContextTLSSlot;
}

bool RemoteThreadContext::Info::IsUsingThreadContextBase()
{
    // Newer builds use ThreadContextBase. Casting to ThreadContext* needs more steps because of multiple inheritence.
    GetExtension()->DetectFeatureBySymbol(m_usingThreadContextBase, GetExtension()->FillModule("%s!ThreadContextBase"));
    return m_usingThreadContextBase;
}

bool RemoteThreadContext::Info::IsUsingTemplatedLinkedList()
{
    if (!m_usingTemplatedLinkedList.HasValue())
    {
        ExtRemoteTyped container = GetFirstThreadContextContainer();

        // This field was removed in changelist 1296403 which was the first change to the structure post blue
        // so before that, we weren't using templated linked lists
        if (container.HasField("objectLiteralId"))
        {
            m_usingTemplatedLinkedList = false;
        }
        else
        {
            // "next" is of type ThreadContext* already with latest templated linked list. Don't need double casts.
            m_usingTemplatedLinkedList = strcmp(container.Field("next").GetTypeName(), "class ThreadContext *") == 0; // This field is renamed in change 1312126, which refactored linked list
        }
    }
    return m_usingTemplatedLinkedList;
}

RemoteThreadContext::Info * RemoteThreadContext::GetInfo()
{
    return &GetExtension()->remoteThreadContextInfo;
}

bool
RemoteThreadContext::IsUsingGlobalListFirst()
{
    return GetInfo()->IsUsingGlobalListFirst();
}

bool
RemoteThreadContext::IsUsingThreadContextTLSEntry()
{
    return GetInfo()->IsUsingThreadContextTLSEntry();
}

bool
RemoteThreadContext::IsUsingThreadContextTLSSlot()
{
    return GetInfo()->IsUsingThreadContextTLSSlot();
}

bool
RemoteThreadContext::IsUsingThreadContextBase()
{
    return GetInfo()->IsUsingThreadContextBase();
}

bool
RemoteThreadContext::IsUsingTemplatedLinkedList()
{
    return GetInfo()->IsUsingTemplatedLinkedList();
}

// Get s_tlsEntryList to enumerate ThreadContexts
ExtRemoteTyped RemoteThreadContext::GetTlsEntryList()
{
    // Newer builds use ThreadContextTLSEntry::s_tlsEntryList, older use ThreadContext::s_tlsEntryList
    Assert(!IsUsingGlobalListFirst());
   
    return ExtRemoteTyped(GetExtension()->FillModule(IsUsingThreadContextTLSEntry() ?
        "%s!ThreadContextTLSEntry::s_tlsEntryList" : "%s!ThreadContext::s_tlsEntryList"));
}

ExtRemoteTyped RemoteThreadContext::GetFirstThreadContextContainer()
{   
    return IsUsingGlobalListFirst() ? ExtRemoteTyped(GetExtension()->FillModule("%s!ThreadContext::globalListFirst")) : GetTlsEntryList();        
}

ExtRemoteTyped RemoteThreadContext::GetNextThreadContextContainer(ExtRemoteTyped container)
{
    return (!IsUsingGlobalListFirst() || IsUsingTemplatedLinkedList()) ? container.Field("next") :
        ExtRemoteTyped(GetExtension()->FillModule2("(%s!ThreadContext *)(%s!JsUtil::DoublyLinkedListElement<ThreadContext> *)@$extin"), container.Field("next").GetPtr());
}

RemoteThreadContext RemoteThreadContext::GetThreadContextFromContainer(ExtRemoteTyped container)
{
    return RemoteThreadContext(!IsUsingGlobalListFirst() ? container.Field("threadContext") : container);
}


#define TLS_EXPANSION_SLOTS 0x400

// Get a TLS slot value from teb
bool RemoteThreadContext::GetTlsSlot(ExtRemoteTyped& teb, ULONG tlsSlotIndex, ULONG64* pValue)
{
    if (tlsSlotIndex < TLS_MINIMUM_AVAILABLE) {
        *pValue = teb.Field("TlsSlots")[tlsSlotIndex].GetPtr();
        return true;
    }
    else if (tlsSlotIndex < TLS_MINIMUM_AVAILABLE + TLS_EXPANSION_SLOTS) {
        ExtRemoteTyped tlsExpansionSlots = teb.Field("TlsExpansionSlots");
        if (tlsExpansionSlots.GetPtr()) {
            tlsExpansionSlots = ExtRemoteTyped("(void **)@$extin", tlsExpansionSlots.GetPtr());
            *pValue = tlsExpansionSlots[tlsSlotIndex - TLS_MINIMUM_AVAILABLE].GetPtr();
            return true;
        }
    }

    return false;
}
// Get JS threadContext, ThreadContext::GetContextForCurrentThread()
bool RemoteThreadContext::TryGetThreadContextFromTeb(RemoteThreadContext& remoteThreadContext)
{
    ExtRemoteTyped teb = ExtRemoteTypedUtil::GetTeb();
    ExtRemoteTyped tlsSlot(GetExtension()->FillModule(IsUsingThreadContextTLSSlot() ?
        "%s!ThreadContextTLSEntry::s_tlsSlot" : "%s!ThreadContext::s_tlsSlot"));
    ULONG tlsSlotIndex = tlsSlot.GetUlong();

    ULONG64 tlsEntryPtr = NULL;
    if (GetTlsSlot(teb, tlsSlotIndex, &tlsEntryPtr) && tlsEntryPtr)
    {
        ExtRemoteTyped tlsEntry(GetExtension()->FillModule("(%s!ThreadContextTLSEntry*)@$extin"), tlsEntryPtr);
        ExtRemoteTyped threadContextId = tlsEntry.Field("threadContext");
        
        if (IsUsingThreadContextBase())
        {
            remoteThreadContext = ExtRemoteTyped(GetExtension()->FillModule2("(%s!ThreadContext*)(%s!ThreadContextBase*)@$extin"), threadContextId.GetPtr());
        }
        else
        {
            remoteThreadContext = threadContextId;
        }
        return true;
    }
   
    // If we are on a different thread, try the thread context list
    ExtRemoteTyped container = GetFirstThreadContextContainer();

    if (container.GetPtr())
    {
        RemoteThreadContext threadContext = GetThreadContextFromContainer(container);

        if (threadContext.GetPtr())
        {
            container = GetNextThreadContextContainer(container);

            if (!container.GetPtr())
            {
                remoteThreadContext = GetThreadContextFromContainer(container);
                return true;
            }
        }
    }
    return false;
}

bool 
RemoteThreadContext::TryGetCurrentThreadContext(RemoteThreadContext& remoteThreadContext)
{    
    if (!HasThreadId())
    {
        return RemoteThreadContext::TryGetThreadContextFromTeb(remoteThreadContext);
    }
    ULONG threadId = 0;
    GetExtension()->m_System4->GetCurrentThreadSystemId(&threadId);
    bool found = RemoteThreadContext::ForEach([threadId, &remoteThreadContext](RemoteThreadContext threadContext)
    {
        if (threadContext.GetThreadId() == threadId)
        {
            remoteThreadContext = threadContext;
            return true;
        }
        return false;
    });

    return found;
}

RemoteThreadContext 
RemoteThreadContext::GetCurrentThreadContext(ULONG64 fallbackRecyclerAddress)
{
    RemoteThreadContext foundThreadContext;
    if (!TryGetCurrentThreadContext(foundThreadContext))
    {
        if (fallbackRecyclerAddress != 0)
        {
            GetExtension()->Out("Warning: thread context not found on current thread, using Recycler's collectionWrapper instead");
            ExtRemoteTyped fallbackThreadContext(GetExtension()->FillModuleAndMemoryNS("(ThreadContext*) ((%s!%sRecycler*) @$extin)->collectionWrapper"), fallbackRecyclerAddress);

            return RemoteThreadContext(fallbackThreadContext);
        }

        GetExtension()->ThrowLastError("Failed to find thread context for current thread. Try using !stst first");
    }
    return foundThreadContext;
}

bool RemoteThreadContext::HasThreadId()
{
    return RemoteThreadContext(ExtRemoteTyped(GetExtension()->FillModule("(%s!ThreadContext *)0"))).HasThreadIdField();
}

bool RemoteThreadContext::HasThreadIdField()
{
    // Win7/8 doesn't have currentThreadId
    return threadContext.HasField("currentThreadId");
}

ULONG RemoteThreadContext::GetThreadId()
{
    return threadContext.Field("currentThreadId").GetUlong();
}

bool RemoteThreadContext::TryGetDebuggerThreadId(ULONG * pDebuggerThreadId, ULONG * threadId)
{
    if (!HasThreadIdField())
    {
        return false;
    }

    ulong threadContextSystemThreadId = this->GetThreadId();
    
    HRESULT hr = g_Ext->m_System4->GetThreadIdBySystemId(threadContextSystemThreadId, pDebuggerThreadId);
    if (SUCCEEDED(hr))
    {
        if (threadId)
        {
            *threadId = threadContextSystemThreadId;
        }
        return true;
    }
    return false;
}

RemoteRecycler RemoteThreadContext::GetRecycler()
{
    return RemoteRecycler(threadContext.Field("recycler"));
}

ULONG64 RemoteThreadContext::GetPtr()
{
    return threadContext.GetPtr();
}

bool RemoteThreadContext::UseCodePageAllocator()
{
    return threadContext.HasField("codePageAllocators");
}

bool
RemoteThreadContext::TryGetThreadContextFromAnyContextPointer(ULONG64 contextPointer, RemoteThreadContext& remoteThreadContext)
{
    return ForEach([&](RemoteThreadContext threadContext)
    {
        if (threadContext.GetPtr() == contextPointer)
        {
            remoteThreadContext = threadContext;
            return true;
        }

        return threadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
        {
            if (scriptContext.GetPtr() == contextPointer)
            {
                remoteThreadContext = scriptContext.GetThreadContext();
                return true;
            }
            return false;
        });
    });
}

bool
RemoteThreadContext::TryGetThreadContextFromPointer(ULONG64 contextPointer, RemoteThreadContext& remoteThreadContext)
{
    return ForEach([&](RemoteThreadContext threadContext)
    {
        if (threadContext.GetPtr() == contextPointer)
        {
            remoteThreadContext = threadContext;
            return true;
        }
        return false;
    });
}

uint
RemoteThreadContext::PrintAll(ulong * pScriptThreadId)
{
    ULONG64 sharedUserDataAddress;
    HRESULT hr;

    if ((hr = g_Ext->m_Data->ReadDebuggerData(
        DEBUG_DATA_SharedUserData, &sharedUserDataAddress,
        sizeof(sharedUserDataAddress), NULL)) != S_OK)
    {
        sharedUserDataAddress = (ULONG64)0;
    }

    ExtRemoteTyped sharedUserData("(nt!_KUSER_SHARED_DATA*)@$extin", sharedUserDataAddress);
    ULONG64 systemTime = ((ULONG64)sharedUserData.Field("SystemTime.High1Time").GetUlong() << 32ull) + sharedUserData.Field("SystemTime.LowPart").GetUlong();
        
    uint numThreadContexts = 0;
    ulong scriptThreadId = 0;
    RemoteThreadContext currentRemoteThreadContext;
    ULONG64 currentThreadContext = 0;
    if (RemoteThreadContext::TryGetCurrentThreadContext(currentRemoteThreadContext))
    {
        currentThreadContext = currentRemoteThreadContext.GetPtr();
    }    
    g_Ext->Out("     Thread Context\n");
    RemoteThreadContext::ForEach([currentThreadContext, systemTime, &scriptThreadId, &numThreadContexts](RemoteThreadContext threadContext)
    {
        ulong threadContextThreadId = 0;

        ULONG64 threadContextAddress = threadContext.GetPtr();
        if (threadContext.TryGetDebuggerThreadId(&threadContextThreadId))
        {
            if (currentThreadContext != threadContextAddress && GetExtension()->PreferDML())
            {
                g_Ext->Dml("<link cmd=\"~%us\">%4u</link>", threadContextThreadId, threadContextThreadId);
            }
            else
            {
                g_Ext->Out("%4u", threadContextThreadId);
            }
            if (GetExtension()->PreferDML())
            {
                g_Ext->Dml(" <link cmd=\"dt %s %p\">%p</link>", GetExtension()->FillModule("%s!ThreadContext"), threadContextAddress, threadContextAddress);
            }
            else
            {
                g_Ext->Out(" %p", threadContextAddress);
            }
        }
        else
        {
            g_Ext->Out("   ? %p", threadContextAddress);
        }


        if (currentThreadContext == threadContextAddress)
        {
            g_Ext->Out(" (current)");
        }
        else
        {
            g_Ext->Out("          ");
        }
        if (threadContext.GetCallRootLevel() != 0)
        {
            ExtRemoteTyped telemetryBlock = threadContext.GetExtRemoteTyped().Field("telemetryBlock");
            ULONG64 lastScriptStartTime = ((ULONG64)telemetryBlock.Field("lastScriptStartTime.dwHighDateTime").GetUlong() << 32ull) + telemetryBlock.Field("lastScriptStartTime.dwLowDateTime").GetUlong();
            g_Ext->Out(" [In script time: %.2fms]", (double)(systemTime - lastScriptStartTime) / 10 / 1000);
        }
        RemoteRecycler recycler = threadContext.GetRecycler();
        if (recycler.CollectionInProgress())
        {
            ExtRemoteTyped telemetryBlock = threadContext.GetRecycler().GetExtRemoteTyped().Field("telemetryBlock");
            ULONG64 lastGCTriggerTime = ((ULONG64)telemetryBlock.Field("currentCollectionStartTime.dwHighDateTime").GetUlong() << 32ull) + telemetryBlock.Field("currentCollectionStartTime.dwLowDateTime").GetUlong();
            if (systemTime < lastGCTriggerTime)
            {
                g_Ext->Out(" [Last GC trigger time: <after system time>]");
            }
            else
            {
                g_Ext->Out(" [Last GC trigger time: %.2fms]", (double)(systemTime - lastGCTriggerTime) / 10 / 1000);
            }
        }
        g_Ext->Out("\n");

        numThreadContexts++;
        scriptThreadId = threadContextThreadId;
        return false;
    });
    if (pScriptThreadId)
    {
        *pScriptThreadId = scriptThreadId;
    }
    return numThreadContexts;
}