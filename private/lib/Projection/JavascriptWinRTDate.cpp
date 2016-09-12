//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "ProjectionPch.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "JavascriptWinRTDate.h"

namespace Js
{
    JavascriptWinRTDate::JavascriptWinRTDate(int64 dateValue, DynamicType* type) 
        : JavascriptDate(0, type), m_dateValue(dateValue)
    {
        Assert(type->GetTypeId() == TypeIds_WinRTDate);

        double es5date;
        
        WinRTDateToES5Date(this->m_dateValue, &es5date);

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
        ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
        ProjectionContext* projectionContext = scriptSite->GetScriptEngine()->GetProjectionContext();
        return RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptWinRTDate, rtDate,
            projectionContext->GetProjectionExternalLibrary()->GetWinRTDateType());
    }

    //
    // Convert a WinRT DateTime date (in 100ns precision ticks) to an ES5 date
    //
    // We convert ticks to milliseconds and shift by JS epoch to get the double date
    // We go in that order to skip doing underflow checks
    //
    HRESULT JavascriptWinRTDate::WinRTDateToES5Date(INT64 ticks, __out double* pRet)
    {
        Assert(pRet != NULL);

        if (pRet == NULL)
        {
            return E_INVALIDARG;
        }

        // Divide as INT64 to ensure truncation of all decimal digits,
        // since any remaining after conversion will be truncated as a Date value.
        INT64 milliseconds = ticks / Js::DateUtilities::ticksPerMillisecond;

        (*pRet) = (double)(milliseconds - Js::DateUtilities::jsEpochMilliseconds);

        return S_OK;
    }

}