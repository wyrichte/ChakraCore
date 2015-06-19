//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

void QueryContinuePoller::TryInterruptPoll(Js::ScriptContext *scriptContext)
{
    // Poll the host on the main thread.

    HostScriptContext *hostScriptContext = scriptContext->GetHostScriptContext();
    if (hostScriptContext == null)
    {
        // No host for this script.
        return;
    }
 
    ScriptSite *scriptSite = ((JScript9HostScriptContext *)hostScriptContext)->GetScriptSite();

    if (scriptSite == null)
    {
        // Report an error?
        return;
    }

    DWORD tickLast = this->lastPollTick;
    DWORD tickNow = ::GetTickCount();
    DWORD timeout = scriptSite->GetTicksPerPoll();

    if (tickNow - tickLast >= timeout)
    {
        this->lastPollTick = tickNow;
        this->DoInterruptPoll(scriptSite);
    }
}

void QueryContinuePoller::DoInterruptPoll(ScriptSite *scriptSite)
{
    if (QueryContinuePoller::GetInterruptPollState(scriptSite) == E_ABORT)
    {
        // Kill the script. This will be caught at the host boundary and translated to E_ABORT.
        throw Js::ScriptAbortException();
    }
}

HRESULT QueryContinuePoller::GetInterruptPollState(ScriptSite *scriptSite)
{
    IActiveScriptSiteInterruptPoll *iPoll = scriptSite->RetrieveInterruptPoll();
    if (iPoll == null)
    {
        // Host doesn't support QC.
        return E_FAIL;
    }

    // Do the poll.
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    HRESULT hr = NOERROR;
    
    // Even though this go to the host, it will never call back us
    // Pass false for leaveForHost, so that this doesn't count as an implicit call
    
    BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext)
    {
        ASYNC_HOST_OPERATION_START(scriptContext->GetThreadContext());

        hr = iPoll->QueryContinue();

        ASYNC_HOST_OPERATION_END(scriptContext->GetThreadContext());
    }
    END_LEAVE_SCRIPT_INTERNAL(scriptContext);
    scriptSite->SetInterruptPoll(iPoll);

    return hr;
}

