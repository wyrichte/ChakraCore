//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    inline bool JavascriptDate::Is(Var aValue)
    {
        // All WinRT Date's are also implicitly Javascript dates
        return IsDateTypeId(JavascriptOperators::GetTypeId(aValue));
    }

    inline JavascriptDate* JavascriptDate::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'Date'");
        
        return static_cast<JavascriptDate *>(RecyclableObject::FromVar(aValue));
    }

    inline Var JavascriptDate::GetDateData(JavascriptDate* date, DateImplementation::DateData dd, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVarIntCheck(date->m_date.GetDateData(dd, false, scriptContext), scriptContext);
    }

    inline Var JavascriptDate::GetUTCDateData(JavascriptDate* date, DateImplementation::DateData dd, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVarIntCheck(date->m_date.GetDateData(dd, true, scriptContext), scriptContext);
    }

    inline Var JavascriptDate::SetDateData(JavascriptDate* date, Arguments args, DateImplementation::DateData dd, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVarNoCheck(date->m_date.SetDateData(args, dd, false, scriptContext), scriptContext);
    }

    inline Var JavascriptDate::SetUTCDateData(JavascriptDate* date, Arguments args, DateImplementation::DateData dd, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVarNoCheck(date->m_date.SetDateData(args, dd, true, scriptContext), scriptContext);
    }

} // namespace Js
