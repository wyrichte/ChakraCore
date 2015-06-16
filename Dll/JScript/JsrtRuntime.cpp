//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "JsrtRuntime.h"

JsrtRuntime::JsrtRuntime(ThreadContext * threadContext, bool useIdle, bool dispatchExceptions)
{       
    Assert(threadContext != NULL);
    this->threadContext = threadContext;
    this->contextList = NULL;
    this->collectCallback = NULL;
    this->beforeCollectCallback = NULL;
    this->callbackContext = NULL;
    this->allocationPolicyManager = threadContext->GetAllocationPolicyManager();
    this->useIdle = useIdle;
    this->dispatchExceptions = dispatchExceptions;
    if (useIdle)
    {
        this->threadService.Initialize(threadContext);
    }
    threadContext->SetJSRTRuntime(this);
}

JsrtRuntime::~JsrtRuntime()
{
    HeapDelete(allocationPolicyManager);
}

// This is called at process detach. 
// threadcontext created from runtime should not be destroyed in ThreadBoundThreadContext
// we should clean them up at process detach only as runtime can be used in other threads
// even after the current physical thread was destroyed. 
// This is called after ThreadBoundThreadContext are cleaned up, so the remaining items
// in the globalthreadContext linklist should be for jsrt only.
void JsrtRuntime::Uninitialize()
{
    ThreadContext* currentThreadContext = ThreadContext::GetThreadContextList();
    ThreadContext* tmpThreadContext;
    while (currentThreadContext)
    {
        Assert(!currentThreadContext->IsScriptActive());
        JsrtRuntime* currentRuntime = static_cast<JsrtRuntime*>(currentThreadContext->GetJSRTRuntime());
        tmpThreadContext = currentThreadContext;
        currentThreadContext = currentThreadContext->Next();
        
        currentRuntime->CloseContexts();
        RentalThreadContextManager::DestroyThreadContext(tmpThreadContext);
        HeapDelete(currentRuntime);
    }
}

void JsrtRuntime::CloseContexts()
{
    while (this->contextList != NULL)
    {
        this->contextList->Dispose(false);
        // This will remove it from the list
    }
}

void JsrtRuntime::SetBeforeCollectCallback(JsBeforeCollectCallback beforeCollectCallback, void * callbackContext)
{
    if (beforeCollectCallback != NULL)
    {        
        if (this->collectCallback == NULL)
        {
            this->collectCallback = this->threadContext->AddRecyclerCollectCallBack(RecyclerCollectCallbackStatic, this);
        }

        this->beforeCollectCallback = beforeCollectCallback;
        this->callbackContext = callbackContext;
    }
    else
    {
        if (this->collectCallback != NULL)
        {
            this->threadContext->RemoveRecyclerCollectCallBack(this->collectCallback);
            this->collectCallback = NULL;
        }

        this->beforeCollectCallback = NULL;
        this->callbackContext = NULL;
    }
}

void JsrtRuntime::RecyclerCollectCallbackStatic(void * context, RecyclerCollectCallBackFlags flags)
{
    if (flags & Collect_Begin)
    {
        JsrtRuntime * _this = reinterpret_cast<JsrtRuntime *>(context);
        try
        {
            _this->beforeCollectCallback(_this->callbackContext);
        }
        catch (...)
        {
            AssertMsg(false, "Unexpected non-engine exception.");
        }
    }
}

unsigned int JsrtRuntime::Idle()
{
    return this->threadService.Idle();
}