//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "ProgressTracker.h"

ProgressTracker::ProgressTracker(char const * message, uint frequency, uint maxIterations) 
    : message(message), frequency(frequency), iter(0), maxIterations(maxIterations), start(GetTickCount64()), last(start)
{
}
void ProgressTracker::Inc()
{
    iter++;
    if (iter % frequency == 0)
    {
        auto curr = GetTickCount64();
        auto timediff = (double)(curr - last) / 1000;
        if (timediff != 0)
        {
            auto speed = (double)iter / timediff;
            double eta = 0;
            if (maxIterations > iter)
            {
                eta = (maxIterations - iter) / speed;
            }
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\r%s - %11d/%11d (%5u/s, ETA: %.2fs)", message, iter, maxIterations, (ULONG)speed, eta);
        }
    }
}

void ProgressTracker::ResetIter(char const * newMessage)
{
    message = newMessage;
    iter = 0;
    last = GetTickCount64();
}

void ProgressTracker::Done(char const * doneMessage)
{
    g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
        "\r%s - elapsed time: %.2fs                                                    \n",
        doneMessage, ((double)(GetTickCount64() - start) / 1000));
}