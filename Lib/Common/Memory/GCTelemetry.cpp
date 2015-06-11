

//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js 
{
    void GCTelemetry::LogGCPauseStartTime()
    {
        if(isHiResAvail)
        {
            QueryPerformanceCounter(&(hiResGCStartTime));
        }
        else
        {
            gcStartTime = GetTickCount64();
        }
    }

    void GCTelemetry::LogGCPauseEndTime()
    {
        allGCPauseCounts++;
        if(isHiResAvail)
        {
            LARGE_INTEGER hiResGCStopTime;
            QueryPerformanceCounter(&hiResGCStopTime);
            gcPauseTime = ((hiResGCStopTime.QuadPart - hiResGCStartTime.QuadPart)* 1000.00/freq.QuadPart);
        }
        else
        {
            ULONGLONG gcStopTime = GetTickCount64(); // get the best estimate when hires isn't available
            gcPauseTime = (double)(gcStopTime - gcStartTime);
        }

        if (isScriptSiteCloseGC == true)
        {
            stats.scriptSiteCloseGCTime = gcPauseTime;
            isScriptSiteCloseGC = false;
            return;
        }
        
        if(gcPauseTime > stats.maxGCPauseTime)
        {
            stats.maxGCPauseTime = gcPauseTime;
        }

        // cohort the GC pauses based on their time
        if(gcPauseTime<3)
        {
            stats.lessThan3MS++;
        }
        else if(gcPauseTime>=3 && gcPauseTime<7)
        {
            stats.within3And7MS++;
        }
        else if(gcPauseTime>=7 && gcPauseTime<10)
        {
            stats.within7And10MS++;
        }
        else if(gcPauseTime>=10 && gcPauseTime<20)
        {
            stats.within10And20MS++;
        }
        else if(gcPauseTime>=20 && gcPauseTime<50)
        {
            stats.within20And50MS++;
        }
        else if (gcPauseTime >= 50 && gcPauseTime < 100)
        {
            stats.within50And100MS++;
        }
        else if (gcPauseTime >= 100 && gcPauseTime < 300)
        {
            stats.within100And300MS++;
        }
        else
        {
            stats.greaterThan300MS++;
        }
        
        if (gcPauseTime >= 50)
        {
            stats.greaterThan50MS++;
        }
        stats.totalGCPauseTime+=gcPauseTime;

    }

    void GCTelemetry::ComputeMean()
    {
        if(allGCPauseCounts>0)
        { 
            stats.meanGCPauseTime = (stats.totalGCPauseTime/ allGCPauseCounts);
        }
    }


}