//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once

class ActiveScriptExternalLibrary : public Js::ExternalLibraryBase
{
public:
    static const unsigned int DOM_BUILTIN_MAX_SLOT_COUNT = 100;

    ActiveScriptExternalLibrary();
    HRESULT Initialize(Js::JavascriptLibrary* library);

    void InitializeDiagnosticsScriptObject(Js::DiagnosticsScriptObject* newDiagnosticsScriptObject);
    Js::DiagnosticsScriptObject* GetDiagnosticsScriptObect() { return diagnosticsScriptObject; }
    Js::StaticType * GetDispMemberProxyType() { return dispMemberProxyType; }
    Js::StaticType * GetHostDispatchType() { return hostDispatchType; }
    Js::DynamicType * GetHostObjectType() { return hostObjectType; }
    void SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke);
    Js::DynamicType * GetModuleRootType() const { return moduleRootType; }
    Js::JavascriptFunction* CreateTypedObjectSlotGetterFunction(unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId);
    Js::JavascriptFunction* CreateTypedObjectSlotSetterFunction(unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId);

private:
    Js::StaticType * dispMemberProxyType;
    Js::StaticType * hostDispatchType;
    Js::DynamicType * hostObjectType;
    Js::DynamicType * moduleRootType;
    Js::DiagnosticsScriptObject * diagnosticsScriptObject;
    Js::DynamicType * typedObjectSlotGetterFunctionTypes[DOM_BUILTIN_MAX_SLOT_COUNT];
    Js::DynamicType * typedObjectSlotSetterFunctionTypes[DOM_BUILTIN_MAX_SLOT_COUNT];

    void InitializeTypes();
    void InitializeDiagnosticsScriptObject();
};
