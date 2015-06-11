//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
// This class is used to communicate between ThreadContext and JavascriptThreadService

class ThreadServiceWrapper abstract
{
public: 
    virtual bool ScheduleNextCollectOnExit() = 0;
    virtual void ScheduleFinishConcurrent() = 0;
    virtual void SetForceOneIdleCollection() = 0;
};

