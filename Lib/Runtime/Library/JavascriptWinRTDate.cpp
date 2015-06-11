//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Common.h"

namespace Js
{
    JavascriptWinRTDate::JavascriptWinRTDate(int64 dateValue, DynamicType* type) 
        : JavascriptDate(0, type), m_dateValue(dateValue)
    {
        Assert(type->GetTypeId() == TypeIds_WinRTDate);

        double es5date;
        
        DateUtilities::WinRTDateToES5Date(this->m_dateValue, &es5date);

        m_date.SetTvUtc(es5date);
    }
}