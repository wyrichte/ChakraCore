#pragma once

namespace Js {

    inline bool JavascriptWinRTDate::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_WinRTDate;
    }

    inline INT64 JavascriptWinRTDate::GetTicks()
    {
        return m_dateValue;
    }

    inline bool JavascriptWinRTDate::AreTicksValid()
    {
        return (m_date.IsModified() == false);
    }

    inline JavascriptWinRTDate* JavascriptWinRTDate::New(INT64 rtDate, ScriptContext* scriptContext)
    {
        return RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptWinRTDate, rtDate, 
                           scriptContext->GetLibrary()->GetWinRTDateType());
    }
} // namespace Js
