// Copyright (C) Microsoft. All rights reserved. 

#include "StdAfx.h"

namespace Js {
    uint64      Tick::s_luFreq;
    uint64      Tick::s_luBegin;

#if DBG
    uint64      Tick::s_DEBUG_luStart   = 0;
    uint64      Tick::s_DEBUG_luSkip    = 0;
#endif

    void Tick::InitType()
    {
        /* CheckWin32( */ QueryPerformanceFrequency((LARGE_INTEGER *) &s_luFreq);
        /* CheckWin32( */ QueryPerformanceCounter((LARGE_INTEGER *) &s_luBegin);

#if DBG
        s_luBegin += s_DEBUG_luStart;
#endif


        //
        // Ensure that we have a sufficient amount of time so that we can handle useful time operations.
        //

        uint64 nSec = _UI64_MAX / s_luFreq;
        if (nSec < 5 * 60)
        {
#if FIXTHIS
            PromptInvalid("QueryPerformanceFrequency() will not provide at least 5 minutes");
            return Results::GenericFailure;
#endif
        }
    }

    Tick Tick::Now()
    {
        // Determine our current time
        uint64 luCurrent = s_luBegin;
        /* Verify( */ QueryPerformanceCounter((LARGE_INTEGER *) &luCurrent);

#if DBG
        luCurrent += s_DEBUG_luStart + s_DEBUG_luSkip;
#endif
        
        // Create a Tick instance, usint our delta since we started tracking time.
        uint64 luDelta = luCurrent - s_luBegin;
        return Tick(luDelta);
    }

    uint64 Tick::ToMicroseconds() const
    {
        //
        // Convert time in microseconds (1 / 1000^2).  Because of the finite precision and wrap-around,
        // this math depends on where the Tick is.
        //

        const uint64 luOneSecUs = (uint64) 1000000;
        const uint64 luSafeTick = _UI64_MAX / luOneSecUs;
        if (m_luTick < luSafeTick)
        {
            //
            // Small enough to convert directly into microseconds.
            //
            
            uint64 luTick = (m_luTick * luOneSecUs) / s_luFreq;
            return luTick;
        }
        else
        {
            //
            // Number is too large, so we need to do this is stages.
            // 1. Compute the number of seconds
            // 2. Convert the remainder
            // 3. Add the two parts together
            //

            uint64 luSec    = m_luTick / s_luFreq;
            uint64 luRemain = m_luTick - luSec * s_luFreq;
            uint64 luTick   = (luRemain * luOneSecUs) / s_luFreq;
            luTick         += luSec * luOneSecUs;

            return luTick;
        }
    }

    int TickDelta::ToMilliseconds() const
    {
        if (*this == Infinite())
        {
            return _I32_MAX;
        }
        
        int64 nTickUs = ToMicroseconds();
        
        int64 lnRound = 500;
        if (nTickUs < 0)
        {
            lnRound = -500;
        }
        
        int64 lnDelta = (nTickUs + lnRound) / ((int64) 1000);
        AssertMsg((lnDelta <= INT_MAX) && (lnDelta >= INT_MIN), "Ensure no overflow");

        return (int) lnDelta;
    }

} 
