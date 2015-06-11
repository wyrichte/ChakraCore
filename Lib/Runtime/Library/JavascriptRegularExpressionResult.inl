//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace Js
{
    __inline void JavascriptRegularExpressionResult::SetMatch(JavascriptArray* arr, const UnifiedRegex::GroupInfo match)
    {
        Assert(JavascriptRegularExpressionResult::Is(arr));
        Assert(!match.IsUndefined());

        ScriptContext* scriptContext = arr->GetScriptContext();
        arr->SetSlot(SetSlotArguments(BuiltInPropertyRecords::index.propertyRecord.GetPropertyId(), IndexIndex, JavascriptNumber::ToVar(match.offset, scriptContext)));        
    }
}
