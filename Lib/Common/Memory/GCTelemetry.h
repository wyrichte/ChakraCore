//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{

    // Anything that needs to be logged as part GCPause Stats should be a part of this structure
    struct GCPauseStats
    {
        uint lessThan3MS;
        uint within3And7MS;
        uint within7And10MS;
        uint within10And20MS;
        uint within20And50MS;
        uint greaterThan50MS;
        uint within50And100MS;
        uint within100And300MS;
        uint greaterThan300MS;
        double meanGCPauseTime;
        double totalGCPauseTime;
        double maxGCPauseTime;
        double scriptSiteCloseGCTime;
    };

    class GCTelemetry
    {

    private:
        bool isHiResAvail;
        LARGE_INTEGER freq;
        LARGE_INTEGER hiResGCStartTime;
        ULONGLONG gcStartTime;
        double gcPauseTime;
        uint allGCPauseCounts;
        GCPauseStats stats;
        void ComputeMean();
        bool isScriptSiteCloseGC;
        
    public:
        GCTelemetry()
        {
            this->Reset();
            QueryPerformanceFrequency(&freq);
            if (freq.QuadPart != 0) 
            {
                isHiResAvail=true;
            }
            else
            {
                isHiResAvail=false;
            }
           isScriptSiteCloseGC=false;
        }
        
        void Reset() 
        { 
            allGCPauseCounts = 0;
            stats.maxGCPauseTime = 0.0;
            stats.meanGCPauseTime = 0.0;
            stats.totalGCPauseTime = 0.0;
            stats.lessThan3MS=0;
            stats.within3And7MS=0;
            stats.within7And10MS=0;
            stats.within10And20MS=0;
            stats.within20And50MS=0;
            stats.greaterThan50MS=0;
            stats.within50And100MS=0;
            stats.within100And300MS=0;
            stats.greaterThan300MS=0;
            stats.scriptSiteCloseGCTime=0.0;
        }

        double GetMaxGC()
        {
            return this->stats.maxGCPauseTime; 
        }

        void LogGCPauseStartTime(); // GC start time on main thread
        void LogGCPauseEndTime(); // current time- GC start time on main thread
        void SetIsScriptSiteCloseGC(bool val)
        {
            isScriptSiteCloseGC=val;
        }
        GCPauseStats GetGCPauseStats()
        {
            this->ComputeMean();
            return this->stats;
        }

    };
} // namespace Js.
