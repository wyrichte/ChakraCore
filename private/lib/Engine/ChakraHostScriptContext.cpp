//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "DOMFastPathInfo.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"
#include "ChakraHostScriptContext.h"

bool 
ChakraHostScriptContext::SetCrossSiteForFunctionType(Js::JavascriptFunction * function)
{
    Js::FunctionInfo* functionInfo = function->GetFunctionInfo();
    const bool useSlotAccessCrossSiteThunk =
        ((functionInfo->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0);
    if (!useSlotAccessCrossSiteThunk)
    {
        return false;
    }

    Assert(!Js::ScriptFunction::Is(function));
    Assert(VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(function));
    Assert((function->GetFunctionInfo()->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0);
    Assert(!function->GetDynamicType()->GetIsShared() 
        || function->GetDynamicType()->GetTypeHandler() == function->GetLibrary()->functionTypeHandler);

    // The Crosssite thunk of the function will be different due to Profiler and debugger
    // we have a table of functonType for all functions of the same slot table. We should
    // avoid changing the original type in the table.
    // Therefore we will change type whether is is shared or not
    function->ChangeType();
    function->SetEntryPoint(DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk);
    return true;
}

#if DBG
bool
ChakraHostScriptContext::IsHostCrossSiteThunk(Js::JavascriptMethod address)
{
    return address == DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk;
}
#endif