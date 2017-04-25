//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    HRESULT FastDOM::GetObjectSlotAccessor(
        __in IActiveScriptDirect* activeScriptDirect,
        __in JavascriptTypeId typeId,
        __in PropertyId nameId,
        __in unsigned int slotIndex,
        __in_opt ScriptMethod getterFallBackEntryPoint,
        __in_opt ScriptMethod setterFallBackEntryPoint,
        __out_opt Var* getter,
        __out_opt Var* setter)
    {
        ScriptEngineBase* scriptEngine = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        HRESULT hr = scriptEngine->GetObjectSlotAccessor(typeId, nameId, slotIndex, getterFallBackEntryPoint, setterFallBackEntryPoint, getter, setter);
        FATAL_ON_FAILED_API_RESULT(hr);
        return hr;
    }

    HRESULT FastDOM::GetTypeSlotAccessor(
        __in IActiveScriptDirect* activeScriptDirect,
        __in JavascriptTypeId typeId,
        __in PropertyId nameId,
        __in unsigned int slotIndex,
        __in_opt ScriptMethod getterFallBackEntryPoint,
        __in_opt ScriptMethod setterFallBackEntryPoint,
        __out_opt Var* getter,
        __out_opt Var* setter)
    {
        ScriptEngineBase* scriptEngine = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        HRESULT hr = scriptEngine->GetTypeSlotAccessor(typeId, nameId, slotIndex, getterFallBackEntryPoint, setterFallBackEntryPoint, getter, setter);
        FATAL_ON_FAILED_API_RESULT(hr);
        return hr;
    }
}
