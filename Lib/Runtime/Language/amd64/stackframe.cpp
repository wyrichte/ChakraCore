//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#if !defined(_M_X64)
#error Amd64StackFrame is not supported on this architecture.
#endif

Js::Amd64StackFrame::Amd64StackFrame()
    : scriptContext(null),
      imageBase(0),
      functionEntry(null),
      currentContext(null),
      hasCallerContext(false),
      callerContext(null),
      addressOfCodeAddr(nullptr)
{
}

Js::Amd64StackFrame::~Amd64StackFrame()
{
    if (currentContext)
    {
        scriptContext->GetThreadContext()->GetAmd64ContextsManager()->Release(currentContext);
    }
}

// InitializeByReturnAddress.
// Parameters:
//  unwindToAddress: specifies the address we need to unwind the stack before any walks can be done.
//                   RtlVirtualUnwind API requires that all unwinds are done within same call stack --
//                   this means that we can't capture context here, go 2 frames back (this ctor -> JavascriptStackWalker ctor),
//                   then create new frames (e.g. for Next()) and unwind stack in them.

bool Js::Amd64StackFrame::InitializeByReturnAddress(void *returnAddress, ScriptContext* scriptContext)
{
    CONTEXT* pair = scriptContext->GetThreadContext()->GetAmd64ContextsManager()->Allocate();
    this->scriptContext = scriptContext;
    this->currentContext = pair;
    this->callerContext = pair + 1;

    this->stackCheckCodeHeight = 
        scriptContext->GetThreadContext()->DoInterruptProbe() ? stackCheckCodeHeightWithInterruptProbe
        : scriptContext->GetThreadContext()->GetIsThreadBound() ? stackCheckCodeHeightThreadBound 
        : stackCheckCodeHeightNotThreadBound;

    // this is the context for the current function
    RtlCaptureContext(currentContext);
    OnCurrentContextUpdated();

    // Unwind stack to the frame wher RIP is the returnAddress
    bool found = SkipToFrame(returnAddress);

    if (!found)
    {
        AssertMsg(FALSE, "Amd64StackFrame: can't intialize: can't unwind the stack to specified unwindToAddress.");
        RtlCaptureContext(currentContext); // Restore trashed context, the best we can do.
    }

    return found;
}

bool Js::Amd64StackFrame::Next()
{
    if (hasCallerContext)
    {
        *currentContext = *callerContext;
        OnCurrentContextUpdated();
        return true;
    }

    EnsureFunctionEntry();
    this->addressOfCodeAddr = this->GetAddressOfReturnAddress();
    if (Next(currentContext, imageBase, functionEntry))
    {
        OnCurrentContextUpdated();
        return true;
    }

    return false;
}

VOID *Js::Amd64StackFrame::GetInstructionPointer()
{
    return (VOID *)currentContext->Rip;
}

void *Js::Amd64StackFrame::GetFrame() const
{
    return (void *)currentContext->Rbp;
}

VOID **Js::Amd64StackFrame::GetArgv()
{
    if (EnsureCallerContext())
    {
        return (VOID **)callerContext->Rsp;
    }

    return null;
}

VOID *Js::Amd64StackFrame::GetReturnAddress()
{
    if (EnsureCallerContext())
    {
        return (VOID *)callerContext->Rip;
    }

    return null;
}

void *Js::Amd64StackFrame::GetAddressOfReturnAddress()
{
    if (EnsureCallerContext())
    {
        return (void*)((VOID **)callerContext->Rsp - 1);
    }

    return null;
}

bool Js::Amd64StackFrame::Next(CONTEXT *context, ULONG64 imageBase, RUNTIME_FUNCTION *functionEntry)
{
    Assert(context);

    VOID *handlerData = null;
    ULONG64 establisherFrame = 0;

    if (!context->Rip)
        return false;

    if (functionEntry)
    {
        RtlVirtualUnwind(0 /* UNW_FLAG_NHANDLER */,
                         imageBase,
                         context->Rip,
                         functionEntry,
                         context,
                         &handlerData,
                         &establisherFrame,
                         null);
    }
    else
    {
        // Leaf frames are not listed in the PDATA section because they
        // don't use the stack.
        // Manually crawl to the next frame.
        context->Rip = (ULONG)(*((ULONG64 **)context->Rsp));
        context->Rsp += 8;
    }

    return true;
}

bool
Js::Amd64StackFrame::SkipToFrame(void * returnAddress)
{
    bool found = false;
    while (Next())
    {
        if (((PVOID)currentContext->Rip) == returnAddress)
        {
            found = true;
            break;
        }
        else if (!ThreadContext::IsOnStack((PVOID)currentContext->Rsp))
        {
            AssertMsg(FALSE, "Amd64StackFrame: while doing initial unwind SP got out of stack.");
            break;
        }
    }
    return found;
}

bool 
Js::Amd64StackFrame::IsInStackCheckCode(void *entry)
{
    void *const codeAddr = GetInstructionPointer();

    return ((size_t(codeAddr) - size_t(entry)) <= this->stackCheckCodeHeight);
}

Js::Amd64ContextsManager::Amd64ContextsManager()
    : curIndex(GENERAL_CONTEXT)
{
}

_Ret_writes_(CONTEXT_PAIR_COUNT)
CONTEXT* Js::Amd64ContextsManager::InternalGet(
    _In_range_(GENERAL_CONTEXT, OOM_CONTEXT) ContextsIndex index)
{
    Assert(index < NUM_CONTEXTS);
    return &contexts[CONTEXT_PAIR_COUNT * index];
}

_Ret_writes_(CONTEXT_PAIR_COUNT)
CONTEXT* Js::Amd64ContextsManager::Allocate()
{
    CONTEXT* pair = NULL;

    switch(curIndex)
    {
    case GENERAL_CONTEXT: //0
        pair = InternalGet(curIndex++);
        Assert(curIndex == OOM_CONTEXT); // Next available is OOM_CONTEXT
        break;

    case OOM_CONTEXT: //1
        pair = HeapNewNoThrowArray(CONTEXT, CONTEXT_PAIR_COUNT);
        if (!pair)
        {
            pair = InternalGet(curIndex++);
            Assert(curIndex == NUM_CONTEXTS); // Used up all stock contexts
        }
        break;

    default:
        AssertMsg(false, "Unexpected usage of JavascriptStackWalker. We run out of CONTEXTs on amd64.");
        Amd64StackWalkerOutOfContexts_fatal_error((ULONG_PTR)this);
    }

    Assert(pair);
    memset(pair, 0, sizeof(CONTEXT) * CONTEXT_PAIR_COUNT);
    return pair;
}

void Js::Amd64ContextsManager::Release(_In_ CONTEXT* contexts)
{
    switch(curIndex)
    {
    case GENERAL_CONTEXT:
        AssertMsg(false, "Unexpected release of CONTEXTs. No contexts allocated.");
        break;

    case OOM_CONTEXT:
        if (contexts != InternalGet(curIndex - 1))
        {
            HeapDeleteArray(CONTEXT_PAIR_COUNT, contexts);
        }
        else
        {
            --curIndex;
            Assert(curIndex == GENERAL_CONTEXT); // GENERAL_CONTEXT is now available
        }
        break;

    case NUM_CONTEXTS:
        AssertMsg(contexts == InternalGet(curIndex - 1), "Invalid CONTEXT releasing sequence. Expect to release stock contexts for OOM.");
        --curIndex;
        Assert(curIndex == OOM_CONTEXT); // OOM_CONTEXT is now available
        break;

    default:
        Assert(false); // Invalid state
        break;
    }
}
