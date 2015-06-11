//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "Exception.h"

namespace Js
{
    bool Exception::RaiseIfScriptActive(ScriptContext *scriptContext, unsigned kind, PVOID returnAddress)
    {
        ThreadContext *threadContext = ThreadContext::GetContextForCurrentThread();

        if (threadContext != null && threadContext->IsScriptActive())
        {
            switch (kind) {
            case ExceptionKind_OutOfMemory:
                JavascriptError::ThrowOutOfMemoryError(scriptContext);
                AssertMsg(returnAddress == NULL, "should not have returnAddress passed in");
                AssertMsg(false, "This shouldn't return");

            case ExceptionKind_StackOverflow:
                JavascriptError::ThrowStackOverflowError(scriptContext, returnAddress);
                AssertMsg(false, "This shouldn't return");

            default:
                AssertMsg(false, "Invalid ExceptionKind");
            }
        }

        return false;
    }

    // Recover/Release unused memory and give it back to OS.
    // The function doesn't throw if the attempt to recover memory fails, in which case it simply does nothing.
    // Useful when running out of memory e.g. for Arena but there is some recycler memory which has been committed but is unusued.
    //
    // static
    void Exception::RecoverUnusedMemory()
    {
        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
        if (threadContext)
        {
            Recycler* threadRecycler = threadContext->GetRecycler();
            if (threadRecycler)
            {
                try
                {
                    threadRecycler->CollectNow<CollectOnRecoverFromOutOfMemory>();
                }
                catch (...)
                {
                    // Technically, exception is a valid scenario: we asked to recover mem, and it couldn't.
                    // Do not let the exception leak out.
                }
            }
        }
    }
}
