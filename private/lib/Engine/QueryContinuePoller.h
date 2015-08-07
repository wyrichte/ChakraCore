//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once 

class QueryContinuePoller : public InterruptPoller
{
public:
    QueryContinuePoller(ThreadContext *tc) : 
        InterruptPoller(tc)
    {
    }

    virtual void TryInterruptPoll(Js::ScriptContext *scriptContext) override;

    void DoInterruptPoll(ScriptSite *scriptSite);
    static HRESULT GetInterruptPollState(ScriptSite *scriptSite);
};
