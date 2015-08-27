//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    //
    // This class encapsulates a dual-store native date object for Windows::Foundation::DateTime
    // objects that are provided by the Windows runtime. These can be created only through projection-
    // they can't be created by a user. They behave exactly the same as JavascriptDates, with one exception
    // If no change has been made to the date, then they preserve their int64 value, so they can be
    // round-tripped to WinRT without any precision loss. This type needed to be created because Javascript
    // dates are doubles with 1ms precision as prescribed by ES5, whereas WinRT dates are INT64 with 100ns
    // precision, and preserving precision when possible was important.
    //
    class JavascriptWinRTDate : public JavascriptDate
    {
    protected:
        int64 m_dateValue;

        DEFINE_VTABLE_CTOR(JavascriptWinRTDate, JavascriptDate);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptWinRTDate); 

    public:
        JavascriptWinRTDate(int64 dateValue, DynamicType* type);
        static bool Is(Var aValue);
        static JavascriptWinRTDate* New(INT64 rtDate, ScriptContext* scriptContext);

        INT64 GetTicks();
        bool AreTicksValid();
        
        static JavascriptDate* FromVar(Var aValue);

    };
}