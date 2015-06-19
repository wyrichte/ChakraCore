//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JsrtThreadService : public ThreadServiceWrapperBase
{
public:
    JsrtThreadService();
    ~JsrtThreadService();

    bool Initialize(ThreadContext *threadContext);
    unsigned int Idle();

    // Does nothing, we don't force idle collection for JSRT
    void SetForceOneIdleCollection() override {}

private:
    bool CanScheduleIdleCollect() override { return true; }
    bool OnScheduleIdleCollect(uint ticks, bool scheduleAsTask) override;
    void OnFinishIdleCollect() override;
    bool ShouldFinishConcurrentCollectOnIdleCallback() override;

    unsigned int nextIdleTick;
};
