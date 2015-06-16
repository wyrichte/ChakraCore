//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "jsrtThreadService.h"

//
// JsrtThreadService
//

JsrtThreadService::JsrtThreadService() :
ThreadServiceWrapperBase(),
nextIdleTick(UINT_MAX)
{
}

JsrtThreadService::~JsrtThreadService()
{
    Shutdown();
}

bool JsrtThreadService::Initialize(ThreadContext *threadContext)
{
    return ThreadServiceWrapperBase::Initialize(threadContext);
}

unsigned int JsrtThreadService::Idle()
{
    unsigned int currentTicks = GetTickCount();

    if (currentTicks >= nextIdleTick)
    {
        IdleCollect();
    }

    return nextIdleTick;
}

bool JsrtThreadService::OnScheduleIdleCollect(uint ticks, bool /* canScheduleAsTask */)
{
    nextIdleTick = GetTickCount() + ticks;
    return true;
}

bool JsrtThreadService::ShouldFinishConcurrentCollectOnIdleCallback()
{
    // For the JsrtThreadService, there is no idle task host
    // so we should always try to finish concurrent on entering 
    // the idle callback
    return true;
}

void JsrtThreadService::OnFinishIdleCollect()
{
    nextIdleTick = UINT_MAX;
}
