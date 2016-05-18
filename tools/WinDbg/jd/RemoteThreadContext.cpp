//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

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
RemoteThreadContext RemoteThreadContext::GetThreadContextFromTeb(ExtRemoteTyped teb)
{
    EXT_CLASS_BASE * ext = GetExtension();
    ExtRemoteTyped tlsSlot(ext->FillModule(IsUsingThreadContextTLSSlot() ?
        "%s!ThreadContextTLSEntry::s_tlsSlot" : "%s!ThreadContext::s_tlsSlot"));
    ULONG tlsSlotIndex = tlsSlot.GetUlong();

    ULONG64 tlsEntryPtr = NULL;
    if (GetTlsSlot(teb, tlsSlotIndex, &tlsEntryPtr) && tlsEntryPtr)
    {
        ExtRemoteTyped tlsEntry(ext->FillModule("(%s!ThreadContextTLSEntry*)@$extin"), tlsEntryPtr);
        ExtRemoteTyped threadContextId = tlsEntry.Field("threadContext");
        
        if (IsUsingThreadContextBase())
        {
            return ExtRemoteTyped(ext->FillModule2("(%s!ThreadContext*)(%s!ThreadContextBase*)@$extin"), threadContextId.GetPtr());
        }

        return threadContextId;
    }
    else
    {
        // If we are on a different thread, try the thread context list
        ExtRemoteTyped container = GetFirstThreadContextContainer();

        if (container.GetPtr())
        {
            RemoteThreadContext threadContext = GetThreadContextFromContainer(container);

            if (threadContext.GetExtRemoteTyped().GetPtr())
            {
                container = GetNextThreadContextContainer(container);

                if (!container.GetPtr())
                {
                    return GetThreadContextFromContainer(container);
                }
            }
        }
    }

    ext->ThrowLastError("Sorry, can't determine which threadContext to use. Please specify threadContext or scriptContext.");
}

bool 
RemoteThreadContext::TryGetCurrentThreadContext(RemoteThreadContext& remoteThreadContext)
{
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
RemoteThreadContext::GetCurrentThreadContext()
{
    RemoteThreadContext foundThreadContext;
    if (!TryGetCurrentThreadContext(foundThreadContext))
    {
        GetExtension()->ThrowLastError("Failed to find thread context for current thread. Try using !stst first");
    }
    return foundThreadContext;
}

ULONG RemoteThreadContext::GetThreadId()
{
    return threadContext.Field("currentThreadId").GetUlong();
}

RemoteRecycler RemoteThreadContext::GetRecycler()
{
    return RemoteRecycler(threadContext.Field("recycler"));
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------