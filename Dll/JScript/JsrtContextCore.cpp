//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "jsrtprivate.h"

DWORD JsrtContext::s_tlsSlot = TLS_OUT_OF_INDEXES;

JsrtContext::JsrtContext(JsrtRuntime * runtime)
{   
    this->runtime = runtime;
    scriptContext = nullptr;

    // Link this new JsrtContext up in the JsrtRuntime's context list
    this->next = runtime->contextList;
    this->previous = nullptr;

    if (runtime->contextList != nullptr)
    {
        Assert(runtime->contextList->previous == nullptr);
        runtime->contextList->previous = this;
    }
    
    runtime->contextList = this;
}

/* static */
JsrtContext *JsrtContext::New(JsrtRuntime * runtime)
{        
    auto result = (JsrtContext *)RecyclerNewFinalizedLeaf(runtime->GetThreadContext()->EnsureRecycler(), JsrtContext, runtime);

    result->EnsureScriptContext();

    return result;
}

/* static */
bool JsrtContext::Initialize()
{
    Assert(s_tlsSlot == TLS_OUT_OF_INDEXES);
    s_tlsSlot = TlsAlloc();
    if (s_tlsSlot == TLS_OUT_OF_INDEXES)
        return false;

    return true;
}

/* static */
void JsrtContext::Uninitialize()
{
    if (s_tlsSlot != TLS_OUT_OF_INDEXES)
        TlsFree(s_tlsSlot);
}


/* static */
JsrtContext * JsrtContext::GetCurrent()
{
    Assert(s_tlsSlot != TLS_OUT_OF_INDEXES);

    return (JsrtContext *)TlsGetValue(s_tlsSlot);
}


/* static */
bool JsrtContext::TrySetCurrent(JsrtContext * context)
{
    Assert(s_tlsSlot != TLS_OUT_OF_INDEXES);

    ThreadContext * threadContext;

    //We are not pinning the context after SetCurrentContext, so if the context is not pinned
    //it might be reclaimed half way during execution. In jsrtshell the runtime was optimized out
    //at time of JsrtContext::Run by the compiler. 
    //The change is to pin the context at setconcurrentcontext, and unpin the previous one. In
    //JsDisposeRuntime we'll reject if current context is active, so that will make sure all
    //contexts are unpinned at time of JsDisposeRuntime.
    if (context != nullptr)
    {            
        threadContext = context->GetRuntime()->GetThreadContext();

        if (!ThreadContextTLSEntry::TrySetThreadContext(threadContext))
        {                
            return false;
        }                  
        threadContext->GetRecycler()->RootAddRef((LPVOID)context);
    }
    else
    {
        if (!ThreadContextTLSEntry::ClearThreadContext(true))
        {
            return false;
        }
    }

    JsrtContext* originalContext = (JsrtContext*) TlsGetValue(s_tlsSlot);
    if (originalContext != nullptr)
    {
        originalContext->GetRuntime()->GetThreadContext()->GetRecycler()->RootRelease((LPVOID) originalContext);
    }

    TlsSetValue(s_tlsSlot, context);
    return true;
}

/* static */
bool JsrtContext::Is(void * ref)
{
    //EDMAURER is this still going to work for it's intended use case now that this type is no longer sealed?
    return VirtualTableInfo<JsrtContext>::HasVirtualTable(ref);
}

void JsrtContext::Finalize(bool isShutdown)
{   
}

void JsrtContext::Dispose(bool isShutdown)
{
    if (nullptr != this->scriptContext)
    {
        Js::ScriptContext::Delete(scriptContext);
        this->scriptContext = nullptr;
        
        // Unlink from JsrtRuntime JsrtContext list
        if (this->previous == nullptr)
        {
            // Have to check this because if we failed while creating, it might
            // never have gotten linked in to the runtime at all.
            if (this->runtime->contextList == this)
            {
                this->runtime->contextList = this->next;
            }
        }
        else
        {
            Assert(this->previous->next == this);
            this->previous->next = this->next;
        }

        if (this->next != nullptr)
        {
            Assert(this->next->previous == this);
            this->next->previous = this->previous;
        }
    }
}

void JsrtContext::Mark(Recycler * recycler)
{
    AssertMsg(false, "Mark called on object that isnt TrackableObject");
}

Js::ScriptContext* JsrtContext::EnsureScriptContext()
{
    if (this->scriptContext != nullptr)
    {
        return this->scriptContext;
    }

    ThreadContext* localThreadContext = this->runtime->GetThreadContext();

    AutoPtr<Js::ScriptContext> newScriptContext(Js::ScriptContext::New(localThreadContext));

    newScriptContext->Initialize();

    this->scriptContext = newScriptContext.Detach();

    Js::JavascriptLibrary *library = scriptContext->GetLibrary();
    Assert(library != nullptr);

    library->GetEvalFunctionObject()->SetEntryPoint(&Js::GlobalObject::EntryEval);
    library->GetFunctionConstructor()->SetEntryPoint(&Js::JavascriptFunction::NewInstance);

    return this->scriptContext;
}
