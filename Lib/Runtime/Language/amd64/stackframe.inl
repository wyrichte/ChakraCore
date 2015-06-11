//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once

__inline void Js::Amd64StackFrame::EnsureFunctionEntry()
{
    if (!functionEntry)
    {
        functionEntry = RtlLookupFunctionEntry(currentContext->Rip, &imageBase, null);
    }
}

__inline bool Js::Amd64StackFrame::EnsureCallerContext()
{
    if (!hasCallerContext)
    {
        EnsureFunctionEntry();
        *callerContext = *currentContext;
        if (Next(callerContext, imageBase, functionEntry))
        {
            hasCallerContext = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

__inline void Js::Amd64StackFrame::OnCurrentContextUpdated()
{
    imageBase = 0;
    functionEntry = null;
    hasCallerContext = false;
}
