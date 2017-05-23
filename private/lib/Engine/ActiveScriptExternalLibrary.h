//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once

class ActiveScriptExternalLibrary : public Js::ExternalLibraryBase
{
public:
    static const unsigned int DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT = 20;
    static const unsigned int DOM_BUILTIN_MAX_TYPE_SLOT_COUNT   = 4;

    ActiveScriptExternalLibrary();
    HRESULT Initialize(Js::JavascriptLibrary* library);

    void InitializeDiagnosticsScriptObject(Js::DiagnosticsScriptObject* newDiagnosticsScriptObject);
    Js::DiagnosticsScriptObject* GetDiagnosticsScriptObect() { return diagnosticsScriptObject; }
    Js::StaticType * GetDispMemberProxyType() { return dispMemberProxyType; }
    Js::StaticType * GetHostDispatchType() { return hostDispatchType; }
    Js::DynamicType * GetHostObjectType() { return hostObjectType; }
    void SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke);
    Js::DynamicType * GetModuleRootType() const { return moduleRootType; }
    Js::JavascriptFunction* CreateSlotGetterFunction(bool isObject, unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId, ScriptMethod fallback);
    Js::JavascriptFunction* CreateSlotSetterFunction(bool isObject, unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId, ScriptMethod fallback);

private:
    Js::StaticType * dispMemberProxyType;
    Js::StaticType * hostDispatchType;
    Js::DynamicType * hostObjectType;
    Js::DynamicType * moduleRootType;
    Js::DiagnosticsScriptObject * diagnosticsScriptObject;
    Js::DynamicType * objectSlotGetterFunctionTypes[DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT];
    Js::DynamicType * objectSlotSetterFunctionTypes[DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT];
    Js::DynamicType * typeSlotGetterFunctionTypes[DOM_BUILTIN_MAX_TYPE_SLOT_COUNT];
    Js::DynamicType * typeSlotSetterFunctionTypes[DOM_BUILTIN_MAX_TYPE_SLOT_COUNT];

    void InitializeTypes();
    void InitializeDiagnosticsScriptObject();
};
