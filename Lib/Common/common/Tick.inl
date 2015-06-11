//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// struct Tick
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    ///----------------------------------------------------------------------------
    ///
    /// Tick::Tick
    ///
    /// Tick() initializes a new Tick instance to an "empty" time.  This instance
    /// must be assigned to another Tick instance or Now() to have value.
    ///
    ///----------------------------------------------------------------------------

    inline
    Tick::Tick()
    {
        m_luTick = 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::Tick
    ///
    /// Tick() initializes a new Tick instance to an specific time, in native
    /// time units.
    ///
    ///----------------------------------------------------------------------------

    inline
    Tick::Tick(
        uint64 luTick)                      // Tick, in internal units
    {
        m_luTick = luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::FromMicroseconds
    ///
    /// FromMicroseconds() returns a Tick instance from a given time, in
    /// microseconds.
    ///
    ///----------------------------------------------------------------------------

    inline Tick
    Tick::FromMicroseconds(
        uint64 luTime)                          // Time, in microseconds
    {
        //
        // Ensure we can convert losslessly.
        //

#if DBG
        const uint64 luMaxTick = _UI64_MAX / s_luFreq;
        AssertMsg(luTime <= luMaxTick, "Ensure time can be converted losslessly");
#endif // DBG


        //
        // Create the Tick
        //
        
        uint64 luTick = luTime * s_luFreq / ((uint64) 1000000);
        return Tick(luTick);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::FromQPC
    ///
    /// FromQPC() returns a Tick instance from a given QPC time.
    ///
    ///----------------------------------------------------------------------------

    inline Tick
    Tick::FromQPC(
        uint64 luTime)                      // Time, in QPC units
    {
        return Tick(luTime - s_luBegin);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::ToQPC
    ///
    /// ToQPC() returns the QPC time for this time instance
    ///
    ///----------------------------------------------------------------------------

    inline uint64
    Tick::ToQPC()
    {
        return (m_luTick + s_luBegin);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator +
    ///
    /// operator +()
    ///
    ///----------------------------------------------------------------------------

    inline Tick
    Tick::operator +(
        TickDelta tdChange                  // RHS TickDelta
        ) const
    {
        return Tick(m_luTick + tdChange.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator -
    ///
    /// operator -()
    ///
    ///----------------------------------------------------------------------------

    inline Tick
    Tick::operator -(
        TickDelta tdChange                  // RHS TickDelta
        ) const
    {
        return Tick(m_luTick - tdChange.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator -
    ///
    /// operator -()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    Tick::operator -(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return TickDelta(m_luTick - timeOther.m_luTick);
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator ==
    ///
    /// operator ==() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator ==(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick == timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator !=
    ///
    /// operator !=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator !=(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick != timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator <
    ///
    /// operator <() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator <(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick < timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator <=
    ///
    /// operator <=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator <=(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick <= timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator >
    ///
    /// operator >() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator >(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick > timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///
    /// Tick::operator >=
    ///
    /// operator >=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    Tick::operator >=(
        Tick timeOther                      // RHS Tick
        ) const
    {
        return m_luTick >= timeOther.m_luTick;
    }


    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// struct TickDelta
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::TickDelta
    ///
    /// TickDelta() initializes a new TickDelta instance to "zero" delta.
    ///
    ///----------------------------------------------------------------------------

    inline
    TickDelta::TickDelta()
    {
        m_lnDelta = 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::TickDelta
    ///
    /// TickDelta() initializes a new TickDelta instance to specific time delta,
    /// in native time units.
    ///
    ///----------------------------------------------------------------------------

    inline
    TickDelta::TickDelta(
        int64 lnDelta)
    {
        m_lnDelta = lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::ToMicroseconds
    ///
    /// ToMicroseconds() returns the time delta, in microseconds.  The time is
    /// rounded to the nearest available whole units.
    ///
    ///----------------------------------------------------------------------------

    int64
    TickDelta::ToMicroseconds() const
    {
        if (*this == Infinite())
        {
            return _I64_MAX;
        }
        
        //
        // Ensure we can convert losslessly.
        //
        
        const int64 lnMinTimeDelta = _I64_MIN / ((int64) 1000000);
        const int64 lnMaxTimeDelta = _I64_MAX / ((int64) 1000000);
        AssertMsg((m_lnDelta <= lnMaxTimeDelta) && (m_lnDelta >= lnMinTimeDelta),
                "Ensure delta can be converted to microseconds losslessly");


        //
        // Compute the microseconds.
        //

        int64 lnFreq = (int64) Tick::s_luFreq;
        int64 lnTickDelta = (m_lnDelta * ((int64) 1000000)) / lnFreq;
        return lnTickDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::FromMicroseconds
    ///
    /// FromMicroseconds() returns a TickDelta instance from a given delta, in
    /// microseconds.
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::FromMicroseconds(
        int64 lnTimeDelta)                  // Time delta, in 1000^2 sec
    {
        AssertMsg(lnTimeDelta != _I64_MAX, "Use Infinite() to create an infinite TickDelta");
        
        //
        // Ensure that we can convert losslessly.
        //

        int64 lnFreq = (int64) Tick::s_luFreq;

#if DBG
        const int64 lnMinTimeDelta = _I64_MIN / lnFreq;
        const int64 lnMaxTimeDelta = _I64_MAX / lnFreq;
        AssertMsg((lnTimeDelta <= lnMaxTimeDelta) && (lnTimeDelta >= lnMinTimeDelta),
                "Ensure delta can be converted to native format losslessly");
#endif // DBG


        //
        // Create the TickDelta
        //

        int64 lnTickDelta = (lnTimeDelta * lnFreq) / ((int64) 1000000);
        TickDelta td(lnTickDelta);

        AssertMsg(td != Infinite(), "Can not create infinite TickDelta");
        return td;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::FromMicroseconds
    ///
    /// FromMicroseconds() returns a TickDelta instance from a given delta, in
    /// microseconds.
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::FromMicroseconds(
        int nTimeDelta)                     // Tick delta, in 1000^2 sec
    {
        AssertMsg(nTimeDelta != _I32_MAX, "Use Infinite() to create an infinite TickDelta");
        
        return FromMicroseconds((int64) nTimeDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::FromMilliseconds
    ///
    /// FromMilliseconds() returns a TickDelta instance from a given delta, in
    /// milliseconds.
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::FromMilliseconds(
        int nTimeDelta)                     // Tick delta, in 1000^1 sec
    {
        AssertMsg(nTimeDelta != _I32_MAX, "Use Infinite() to create an infinite TickDelta");
        
        return FromMicroseconds(((int64) nTimeDelta) * ((int64) 1000));
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::Infinite
    ///
    /// Infinite() returns a time-delta infinitely far away.
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::Infinite()
    {
        return TickDelta(_I64_MAX);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::IsForward
    ///
    /// IsForward() returns whether adding this TickDelta to a given Tick will
    /// not move the time backwards.
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::IsForward() const
    {
        return m_lnDelta >= 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::IsBackward
    ///
    /// IsBackward() returns whether adding this TickDelta to a given Tick will
    /// not move the time forwards.
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::IsBackward() const
    {
        return m_lnDelta <= 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::Abs
    ///
    /// Abs() returns the absolute value of the TickDelta.
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::Abs(TickDelta tdOther)
    {
        return TickDelta(tdOther.m_lnDelta < 0 ? -tdOther.m_lnDelta : tdOther.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator %
    ///
    /// operator %()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator %(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return TickDelta(m_lnDelta % tdOther.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator \
    ///
    /// operator \() - Divides one TickDelta by another, in TickDelta units
    ///
    ///----------------------------------------------------------------------------

    inline int64
    TickDelta::operator /(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta / tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator +
    ///
    /// operator +()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator +(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        AssertMsg((*this != Infinite()) && (tdOther != Infinite()),
                "Can not combine infinite TickDeltas");
        
        return TickDelta(m_lnDelta + tdOther.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator -
    ///
    /// operator -()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator -(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        AssertMsg((*this != Infinite()) && (tdOther != Infinite()),
                "Can not combine infinite TickDeltas");
        
        return TickDelta(m_lnDelta - tdOther.m_lnDelta);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator *
    ///
    /// operator *()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator *(
        int nScale                          // RHS scale
        ) const
    {
        AssertMsg(*this != Infinite(), "Can not combine infinite TickDeltas");
        
        return TickDelta(m_lnDelta * nScale);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator *
    ///
    /// operator *()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator *(
        float flScale                       // RHS scale
        ) const
    {
        AssertMsg(*this != Infinite(), "Can not combine infinite TickDeltas");
        
        return TickDelta((int64) (((double) m_lnDelta) * ((double) flScale)));
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator /
    ///
    /// operator /()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator /(
        int nScale                          // RHS scale
        ) const
    {
        AssertMsg(*this != Infinite(), "Can not combine infinite TickDeltas");
        AssertMsg(nScale != 0, "Can not scale by 0");
        
        return TickDelta(m_lnDelta / nScale);
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator /
    ///
    /// operator /()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator /(
        float flScale                       // RHS scale
        ) const
    {
        AssertMsg(*this != Infinite(), "Can not combine infinite TickDeltas");
        AssertMsg(flScale != 0, "Can not scale by 0");
        
        return TickDelta((int64) (((double) m_lnDelta) / ((double) flScale)));
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator +=
    ///
    /// operator +=()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator +=(
        TickDelta tdOther)                  // RHS TickDelta
    {
        AssertMsg((*this != Infinite()) && (tdOther != Infinite()),
                "Can not combine infinite TickDeltas");
        
        m_lnDelta = m_lnDelta + tdOther.m_lnDelta;

        return *this;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator -=
    ///
    /// operator -=()
    ///
    ///----------------------------------------------------------------------------

    inline TickDelta
    TickDelta::operator -=(
        TickDelta tdOther)                  // RHS TickDelta
    {
        AssertMsg((*this != Infinite()) && (tdOther != Infinite()),
                "Can not combine infinite TickDeltas");
        
        m_lnDelta = m_lnDelta - tdOther.m_lnDelta;

        return *this;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator ==
    ///
    /// operator ==() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator ==(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta == tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator !=
    ///
    /// operator !=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator !=(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta != tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator <
    ///
    /// operator <() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator <(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta < tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator <=
    ///
    /// operator <=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator <=(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta <= tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator >
    ///
    /// operator >() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator >(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta > tdOther.m_lnDelta;
    }


    ///----------------------------------------------------------------------------
    ///
    /// TickDelta::operator >=
    ///
    /// operator >=() 
    ///
    ///----------------------------------------------------------------------------

    inline bool
    TickDelta::operator >=(
        TickDelta tdOther                   // RHS TickDelta
        ) const
    {
        return m_lnDelta >= tdOther.m_lnDelta;
    }

} // namespace Js
