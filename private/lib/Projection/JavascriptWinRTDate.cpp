//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Common.h"
#include "JavascriptWinRTDate.h"

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

    bool JavascriptWinRTDate::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_WinRTDate;
    }

    INT64 JavascriptWinRTDate::GetTicks()
    {
        return m_dateValue;
    }

    bool JavascriptWinRTDate::AreTicksValid()
    {
        return (m_date.IsModified() == false);
    }

    JavascriptWinRTDate* JavascriptWinRTDate::New(INT64 rtDate, ScriptContext* scriptContext)
    {
        return RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptWinRTDate, rtDate,
            scriptContext->GetLibrary()->GetWinRTDateType());
    }
}